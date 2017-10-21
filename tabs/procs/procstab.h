#ifndef PROCS_H
#define PROCS_H

#include "tabwidget.h"
#include "procstablecolumn.h"
#include <cstdint>
#include <cstddef>
#include <sys/types.h>
#include <unistd.h>
#include <QHash>

class QTreeWidgetItem;
class ProcessTreeWidgetItem;

struct Task;

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

private slots:
    void customContextMenu(const QPoint& pos);
    void terminateShortcutActivated();

private:
    Task makeFreshTask(pid_t pid, pid_t tgid, uid_t uid, Task* lastTask);
    ProcessTreeWidgetItem *makeTaskWidget(Task& task, QHash<pid_t, Task> &curTasks);
    void updateUptime();
    void readUsers();

public:
    static constexpr const char* name() {return "Processes";}
    static constexpr const ProcsTableColumn columns[] = {
        { "Name", ProcsTableColumn::StringSort, QHeaderView::Stretch, 250 },
        { "PID", ProcsTableColumn::NumberSort, QHeaderView::Interactive, 50 },
        { "User", ProcsTableColumn::UserSort, QHeaderView::Interactive, 100 },
        { "CPU", ProcsTableColumn::NumberSort, QHeaderView::Interactive, 75 },
        { "Memory", ProcsTableColumn::NumberSort, QHeaderView::Interactive, 100 },
    };
private:
    static constexpr const bool showThreads = true; /// TODO: Make this a checkbox in the View menu
    Ui::ProcsTab *ui;
    QHash<pid_t, Task> tasks;
    QHash<uid_t, QString> users;
    unsigned hertz;
    float uptime, lastUptime;
};

#endif // PROCS_H
