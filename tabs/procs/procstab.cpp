#include "procstab.h"
#include "ui_procstab.h"
#include "processtreewidgetitem.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStringRef>
#include <QMenu>
#include <stdexcept>
#include <signal.h>

struct Task
{
    QString name;
    ProcessTreeWidgetItem* widget;
    size_t memAnon;
    uid_t uid;
    pid_t pid;
    pid_t ppid;
    pid_t tgid;
    size_t numThreads;

    size_t utime; // User time
    size_t stime; // Kernerl time
    size_t startTime;
    float cpuPercent;

    ProcessTreeWidgetItem::SortableItems makeTaskWidgetItems(const QHash<uid_t, QString>& users) {
        return {
            {
                name,
                pid,
                uid,
                cpuPercent,
                (qulonglong) memAnon,
            },
            {
                name,
                QString::number(pid),
                users[uid],
                QStringLiteral("%1 %").arg((int)cpuPercent),
                QStringLiteral("%1 MiB").arg(memAnon/1024./1024., 0, 'f', 1),
            }
        };
    }
};

constexpr const ProcsTableColumn ProcsTab::columns[];

ProcsTab::ProcsTab()
    : ui(new Ui::ProcsTab)
{
    ui->setupUi(this);

    QStringList headerLabels;
    for (ProcsTableColumn column : columns)
        headerLabels += column.name;
    ui->treeWidget->setHeaderLabels(headerLabels);
    ui->treeWidget->sortByColumn(3, Qt::AscendingOrder); // CPU
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this, &ProcsTab::customContextMenu);

    auto header = ui->treeWidget->header();
    for (int i=ui->treeWidget->columnCount() - 1; i >= 0; --i) {
        header->resizeSection(i, columns[i].defaultWidth);
        header->setSectionResizeMode(i, columns[i].resizeMode);
    }

    hertz = sysconf(_SC_CLK_TCK);
    uptime = 0;
    updateUptime();
    readUsers();
}

ProcsTab::~ProcsTab()
{
    delete ui;
}

void ProcsTab::refresh()
{
    if (!isVisible())
        return;

    QHash<pid_t, Task> curTasks;
    QVector<pid_t> newTasks;
    updateUptime();

    ui->treeWidget->setSortingEnabled(false);

    // Process all existing tasks
    const auto entries = QDir(QStringLiteral("/proc")).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (QFileInfo entry : entries) try {
        bool isNumber;
        pid_t pid = entry.fileName().toLongLong(&isNumber);
        if (!isNumber || pid <= 0)
            continue;

        auto lastTaskIt = tasks.find(pid);
        Task* lastTask = lastTaskIt == tasks.end() ? nullptr : &lastTaskIt.value();
        auto taskIt = curTasks.insert(pid, makeFreshTask(pid, pid, entry.ownerId(), lastTask));
        if (lastTask)
            tasks.erase(lastTaskIt);
        else
            newTasks += pid;

        if (showThreads && taskIt.value().numThreads > 1) {
            auto entries = QDir(entry.filePath()+"/task").entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (QFileInfo threadEntry : entries) {
                pid_t tid = threadEntry.fileName().toLongLong();
                if (pid == tid)
                    continue;

                auto lastTaskIt = tasks.find(tid);
                Task* lastTask = lastTaskIt == tasks.end() ? nullptr : &lastTaskIt.value();
                curTasks.insert(tid, makeFreshTask(tid, pid, threadEntry.ownerId(), lastTask));
                if (lastTask)
                    tasks.erase(lastTaskIt);
                else
                    newTasks += tid;
            }
        }
    } catch (const std::exception& e) {
        qDebug() << "Failed to read process info for PID"<<entry.fileName()<<":"<<e.what();
        continue;
    }

    // Add new tasks to the view
    for (auto taskPid : newTasks) {
        Task& task = curTasks[taskPid];
        makeTaskWidget(task, curTasks);
    }

    // Delete dead tasks
    for (Task deadTask : tasks) {
        if (!deadTask.widget)
            continue;
        deadTask.widget->takeChildren();
        QTreeWidgetItem* parent = deadTask.widget->parent();
        if (parent)
            parent->removeChild(deadTask.widget);
        else if (auto index = ui->treeWidget->indexOfTopLevelItem(deadTask.widget); index != -1)
            ui->treeWidget->takeTopLevelItem(index);
        delete deadTask.widget;
        deadTask.widget = nullptr;
    }
    tasks = curTasks;

    ui->treeWidget->setSortingEnabled(true);
}

void ProcsTab::customContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->treeWidget->indexAt(pos);
    if (!index.isValid())
        return;
    int pid = index.sibling(index.row(), 1).data().toInt();

    QMenu contextMenu;
    contextMenu.addAction("Terminate process", [this, pid]{
        kill(pid, SIGTERM);
    });
    contextMenu.addAction("Kill process", [this, pid]{
        kill(pid, SIGKILL);
    });

    contextMenu.exec(ui->treeWidget->viewport()->mapToGlobal(pos));
}

Task ProcsTab::makeFreshTask(pid_t pid, pid_t tgid, uid_t uid, Task* lastTask)
{
    Task task;
    task.pid = pid;
    task.tgid = tgid;
    task.uid = uid;

    QString statPath = pid == tgid ? // For the main thread, we report its CPU as the whole process' CPU
                QString("/proc/%2/stat").arg(pid)
              : QString("/proc/%2/task/%2/stat").arg(pid);
    QFile statFile(statPath);
    if (!statFile.open(QIODevice::ReadOnly | QIODevice::Unbuffered))
        throw std::runtime_error(("Failed to read "+statPath).toStdString());

    QString statStr = statFile.readAll();
    auto leftParen = statStr.indexOf('('), rightParen = statStr.lastIndexOf(')');
    if (leftParen == -1 || rightParen == -1 || rightParen < leftParen)
        throw std::runtime_error(QString("Failed to parse name in stat line of process %1").arg(pid).toStdString());

    pid_t parsedPid = QStringRef(&statStr, 0, leftParen).toLongLong();
    if (parsedPid != pid)
        throw std::runtime_error(QString("Stat file says task %1 has pid %2, what is going on!?").arg(pid).arg(task.pid).toStdString());

    task.name = statStr.mid(leftParen+1, rightParen-leftParen-1);

    QStringRef remaining = QStringRef(&statStr, rightParen+2, statStr.size()-rightParen-2);
    auto consume = [&remaining](int skip = 0) -> long long unsigned {
        while (skip--)
            remaining = remaining.mid(remaining.indexOf(' ')+1);

        auto next = remaining.indexOf(' ')+1;
        auto result = remaining.left(next).toULongLong();
        remaining = remaining.mid(next);
        return result;
    };
    task.ppid = consume(1);
    task.utime = consume(9);
    task.stime = consume();
    task.numThreads = consume(4);
    task.startTime = consume(1);
    task.memAnon = consume(1)*4096;

    if (lastTask) {
        size_t runtimeTicks = (task.utime+task.stime) - (lastTask->utime+lastTask->stime);
        float runtime = uptime - lastUptime;
        task.cpuPercent = 100. * runtimeTicks / hertz / runtime;
        task.widget = lastTask->widget;
        task.widget->setItems(task.makeTaskWidgetItems(users));
    } else {
        size_t totalTime = task.utime+task.stime;
        float runtime = uptime - (task.startTime / hertz);
        task.cpuPercent = 100. * totalTime / hertz / runtime;
        task.widget = nullptr;
    }

    return task;
}

void ProcsTab::updateUptime()
{
    QFile uptimeFile(QStringLiteral("/proc/uptime"));
    if (!uptimeFile.open(QIODevice::ReadOnly))
        return;
    QString upStr = uptimeFile.readAll();
    lastUptime = uptime;
    uptime = upStr.left(upStr.indexOf(' ')).toFloat();
}

void ProcsTab::readUsers()
{
    QFile passwd(QStringLiteral("/etc/passwd"));
    if (!passwd.open(QIODevice::ReadOnly)) {
        qWarning() << "Can't open /etc/passwd to read users list!";
        return;
    }
    QList<QString> lines = QString::fromUtf8(passwd.readAll()).split('\n');
    for (QString line : lines) {
        QList<QString> entries = line.split(':');
        if (entries.size() < 3)
            continue;
        users[entries[2].toLongLong()] = entries[0];
    }
}

ProcessTreeWidgetItem *ProcsTab::makeTaskWidget(Task &task, QHash<pid_t, Task>& curTasks)
{
    if (!task.widget) {
        if (task.tgid == task.pid || !task.tgid) {
            task.widget = new ProcessTreeWidgetItem(ui->treeWidget, task.makeTaskWidgetItems(users));
        } else {
            if (!showThreads)
                return nullptr;

            QTreeWidgetItem* parent = makeTaskWidget(curTasks[task.tgid], curTasks);
            task.widget = new ProcessTreeWidgetItem(parent, task.makeTaskWidgetItems(users));
        }
    }

    return task.widget;
}
