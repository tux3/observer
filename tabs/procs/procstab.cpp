#include "procstab.h"
#include "ui_procstab.h"
#include <QDebug>
#include <QDir>
#include <QFile>

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
    QHash<pid_t, Task> curTasks;
    QVector<pid_t> newTasks;
    updateUptime();

    ui->treeWidget->setSortingEnabled(false);

    // Process all existing tasks
    const auto entries = QDir(PROC_PATH).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
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
        qWarning() << "Failed to read process info for PID"<<entry.fileName()<<":"<<e.what();
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

Task ProcsTab::makeFreshTask(pid_t pid, pid_t tgid, uid_t uid, Task* lastTask)
{
    Task task;
    task.pid = pid;
    task.tgid = tgid;
    task.uid = uid;

    QString statPath = pid == tgid ? // For the main thread, we report its CPU as the whole process' CPU
                QString("%1%2/stat").arg(PROC_PATH).arg(pid)
              : QString("%1%2/task/%2/stat").arg(PROC_PATH).arg(pid);
    QFile statFile(statPath);
    if (!statFile.open(QIODevice::ReadOnly))
        throw std::runtime_error(("Failed to read "+statPath).toStdString());

    QString statStr = statFile.readAll();
    auto leftParen = statStr.indexOf('('), rightParen = statStr.lastIndexOf(')');
    if (leftParen == -1 || rightParen == -1 || rightParen < leftParen)
        throw std::runtime_error(QString("Failed to parse name in stat line of process %1").arg(pid).toStdString());

    pid_t parsedPid = statStr.left(leftParen).toLongLong();
    if (parsedPid != pid)
        throw std::runtime_error(QString("Stat file says task %1 has pid %2, what is going on!?").arg(pid).arg(task.pid).toStdString());

    QList<QString> trailingParams = statStr.mid(rightParen+2).split(' ');
    task.name = statStr.mid(leftParen+1, rightParen-leftParen-1);
    task.ppid = trailingParams[1].toLongLong();
    task.utime = trailingParams[11].toLongLong();
    task.stime = trailingParams[12].toLongLong();
    task.cutime = trailingParams[13].toLongLong();
    task.cstime = trailingParams[14].toLongLong();
    task.numThreads = trailingParams[17].toLongLong();
    task.startTime = trailingParams[19].toLongLong();
    task.memAnon = trailingParams[21].toLongLong()*4096;

    if (lastTask) {
        size_t runtimeTicks = (task.utime+task.stime) - (lastTask->utime+lastTask->stime);
        float runtime = uptime - lastUptime;
        task.cpuPercent = 100. * runtimeTicks / hertz / runtime;
        task.widget = lastTask->widget;
        task.widget->setItems(makeTaskWidgetItems(task));
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
    QFile uptimeFile(QString(PROC_PATH)+"uptime");
    if (!uptimeFile.open(QIODevice::ReadOnly))
        return;
    QString upStr = uptimeFile.readAll();
    lastUptime = uptime;
    uptime = upStr.left(upStr.indexOf(' ')).toFloat();
}

void ProcsTab::readUsers()
{
    QFile passwd("/etc/passwd");
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

QVector<ProcessTreeWidgetItem::SortableItem> ProcsTab::makeTaskWidgetItems(const Task &task)
{
    using Item = ProcessTreeWidgetItem::SortableItem;
    Item name = { task.name, task.name };
    Item pid = { task.pid, QString::number(task.pid) };
    Item user = { task.uid, users[task.uid] };
    Item cpu = { task.cpuPercent, QString("%1 %").arg(task.cpuPercent, 0, 'f', 0) };
    Item memory = { (qulonglong) task.memAnon, QString("%1 MiB").arg(task.memAnon/1024./1024., 0, 'f', 1) };

    return {name, pid, user, cpu, memory};
}

ProcessTreeWidgetItem *ProcsTab::makeTaskWidget(Task &task, QHash<pid_t, Task>& curTasks)
{
    if (!task.widget) {
        if (task.tgid == task.pid || !task.tgid) {
            task.widget = new ProcessTreeWidgetItem(ui->treeWidget, makeTaskWidgetItems(task));
        } else {
            if (!showThreads)
                return nullptr;

            QTreeWidgetItem* parent = makeTaskWidget(curTasks[task.tgid], curTasks);
            task.widget = new ProcessTreeWidgetItem(parent, makeTaskWidgetItems(task));
        }
    }

    return task.widget;
}
