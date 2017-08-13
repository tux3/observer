#include "processtreewidgetitem.h"
#include "procstablecolumn.h"
#include "procstab.h"
#include <cassert>

ProcessTreeWidgetItem::ProcessTreeWidgetItem(QTreeWidget *parent, QVector<SortableItem> items)
    : QTreeWidgetItem(parent)
{
    setItems(items);
}

ProcessTreeWidgetItem::ProcessTreeWidgetItem(QTreeWidgetItem *parent, QVector<SortableItem> items)
    : QTreeWidgetItem(parent)
{
    setItems(items);
}

void ProcessTreeWidgetItem::setItems(QVector<ProcessTreeWidgetItem::SortableItem> items)
{
    assert(items.size() == std::extent<decltype(ProcsTab::columns)>());

    for (int i = 0; i < items.size(); ++i) {
        setText(i, items[i].displayText);
        setData(i, Qt::UserRole, items[i].sortKey);
        setSizeHint(i, QSize{QTreeWidgetItem::sizeHint(i).width(), 25});
    }
}

bool ProcessTreeWidgetItem::operator<(const QTreeWidgetItem &other) const {
    int sortColumn = treeWidget()->sortColumn();
    ProcsTableColumn column = ProcsTab::columns[sortColumn];
    const auto& left = other.data(sortColumn, Qt::UserRole);
    const auto& right = data(sortColumn, Qt::UserRole);
    return column.compareRows(left, right);
}
