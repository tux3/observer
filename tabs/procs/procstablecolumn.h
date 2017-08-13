#ifndef PROCSTABLECOLUMN_H
#define PROCSTABLECOLUMN_H

#include <QHeaderView>

class QVariant;

class ProcsTableColumn
{
public:
    enum ColumnType {
        StringSort = 0,
        NumberSort = 1,
        UserSort = 2,
    };

public:
    bool compareRows(const QVariant& left, const QVariant& right);

public:
    const char* const name;
    const ColumnType type;
    const QHeaderView::ResizeMode resizeMode;
    const unsigned defaultWidth;

private:
    static bool compareVariants(const QVariant& left, const QVariant& right);
    static bool compareUsers(const QVariant& left, const QVariant& right);

    using Comparator = bool(*)(const QVariant&, const QVariant&);
    static constexpr const Comparator comparatorFunctions[] = {
        compareVariants,
        compareVariants,
        compareUsers,
    };
};

#endif // PROCSTABLECOLUMN_H
