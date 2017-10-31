#include "processtreewidgetitem.h"
#include "procstablecolumn.h"
#include <cassert>

ProcessTreeWidgetItem::ProcessTreeWidgetItem(QTreeWidget* parent, SortableItems items)
    : QTreeWidgetItem(parent)
{
    setItems(std::move(items));
    setFixedRowHeight();
}

ProcessTreeWidgetItem::ProcessTreeWidgetItem(QTreeWidgetItem* parent, SortableItems items)
    : QTreeWidgetItem(parent)
{
    setItems(std::move(items));
    setFixedRowHeight();
}

void ProcessTreeWidgetItem::setItems(SortableItems items)
{
    assert(items.displayText.size() == std::extent<decltype(ProcsTab::columns)>());

    for (int i = 0; i < items.displayText.size(); ++i)
        setData(i, Qt::DisplayRole, items.displayText[i]);
    sortKeys = std::move(items.sortKeys);
}

bool ProcessTreeWidgetItem::operator<(const QTreeWidgetItem& otherBase) const
{
    const ProcessTreeWidgetItem& otherDerived = reinterpret_cast<const ProcessTreeWidgetItem&>(otherBase);

    int sortColumn = treeWidget()->sortColumn();
    ProcsTableColumn column = ProcsTab::columns[sortColumn];
    const auto& left = otherDerived.sortKeys[sortColumn];
    const auto& right = sortKeys[sortColumn];
    return column.compareRows(left, right);
}

void ProcessTreeWidgetItem::setFixedRowHeight()
{
    QVariant sizeHint = QSize{ QTreeWidgetItem::sizeHint(0).width(), 25 };
    for (int i = 0; i < sortKeys.size(); ++i)
        setData(i, Qt::SizeHintRole, sizeHint);
}
