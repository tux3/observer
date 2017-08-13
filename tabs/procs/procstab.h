#ifndef PROCS_H
#define PROCS_H

#include "tabwidget.h"
#include "procstablecolumn.h"
#include "processtreewidgetitem.h"
#include <cstdint>
#include <cstddef>
#include <sys/types.h>
#include <unistd.h>
#include <QHash>

class QTreeWidgetItem;

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
    // Times, including children
    size_t cutime;
    size_t cstime;
    size_t startTime;
    float cpuPercent;
};

namespace Ui {
class ProcsTab;
}

struct Task;

class ProcsTab : public TabWidget
{
    Q_OBJECT

public:
    ProcsTab();
    ~ProcsTab();

protected slots:
    void refresh();

private:
    Task makeFreshTask(pid_t pid, pid_t tgid, uid_t uid, Task* lastTask);
    QVector<ProcessTreeWidgetItem::SortableItem> makeTaskWidgetItems(const Task& task);
    ProcessTreeWidgetItem *makeTaskWidget(Task& task, QHash<pid_t, Task> &curTasks);
    void updateUptime();
    void readUsers();

public:
    static constexpr const char* name = "Processes";
    static constexpr const ProcsTableColumn columns[] = {
        { "Name", ProcsTableColumn::StringSort, QHeaderView::Stretch, 250 },
        { "PID", ProcsTableColumn::NumberSort, QHeaderView::Interactive, 50 },
        { "User", ProcsTableColumn::UserSort, QHeaderView::Interactive, 100 },
        { "CPU", ProcsTableColumn::NumberSort, QHeaderView::Interactive, 75 },
        { "Memory", ProcsTableColumn::NumberSort, QHeaderView::Interactive, 100 },
    };
private:
    static constexpr const char* PROC_PATH = "/proc/";
    static constexpr const bool showThreads = true; /// TODO: Make this a checkbox in the View menu
    Ui::ProcsTab *ui;
    QHash<pid_t, Task> tasks;
    QHash<uid_t, QString> users;
    unsigned hertz;
    float uptime, lastUptime;
};

#endif // PROCS_H
