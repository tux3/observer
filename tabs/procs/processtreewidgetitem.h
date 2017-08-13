#ifndef PROCESSTREEWIDGETITEM_H
#define PROCESSTREEWIDGETITEM_H

#include "procstab.h"
#include <QTreeWidgetItem>

/**
 * NOTE: Never mix a ProcessTreeWidgetItem with other classes of items in the same column,
 * or if you do don't try to sort them, the comparison operator will hapilly crash on you!
 * Since ProcessTreeWidgetItems are meant to be sorted differently, it wouldn't make sense.
 */
class ProcessTreeWidgetItem : public QTreeWidgetItem
{
public:
    struct SortableItems {
        QVector<QVariant> sortKeys;
        QVector<QVariant> displayText;
    };

public:
    ProcessTreeWidgetItem(QTreeWidget* parent, SortableItems items);
    ProcessTreeWidgetItem(QTreeWidgetItem* parent, SortableItems items);

    void setItems(SortableItems items);

    virtual bool operator<(const QTreeWidgetItem &other) const override;

private:
    void setFixedRowHeight();

private:
    QVector<QVariant> sortKeys;
};

#endif // PROCESSTREEWIDGETITEM_H
