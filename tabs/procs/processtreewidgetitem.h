#ifndef PROCESSTREEWIDGETITEM_H
#define PROCESSTREEWIDGETITEM_H

#include <QTreeWidgetItem>

class ProcessTreeWidgetItem : public QTreeWidgetItem
{
public:
    struct SortableItem {
        QVariant sortKey;
        QString displayText;
    };

public:
    ProcessTreeWidgetItem(QTreeWidget* parent, QVector<SortableItem> items);
    ProcessTreeWidgetItem(QTreeWidgetItem* parent, QVector<SortableItem> items);

    void setItems(QVector<SortableItem> items);

    virtual bool operator<(const QTreeWidgetItem &other) const override;
};

#endif // PROCESSTREEWIDGETITEM_H
