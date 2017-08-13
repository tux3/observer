#include "procstablecolumn.h"
#include <QVariant>

constexpr const ProcsTableColumn::Comparator ProcsTableColumn::comparatorFunctions[];

bool ProcsTableColumn::compareRows(const QVariant &left, const QVariant &right)
{
    Comparator cmp = comparatorFunctions[type];
    return cmp(left, right);
}

bool ProcsTableColumn::compareVariants(const QVariant &left, const QVariant &right)
{
    return left < right;
}

bool ProcsTableColumn::compareUsers(const QVariant &left, const QVariant &right)
{
    uid_t leftUser = left.toLongLong();
    uid_t rightUser = right.toLongLong();

    // Current user is always sorted at the top

    return leftUser < rightUser;
}
