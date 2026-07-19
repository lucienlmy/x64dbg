// This file implements accessibility interface for the column headers of AbstractTableView
#ifndef QT_NO_ACCESSIBILITY
#include "AccessibleAbstractTableViewCellTitle.h"
#include "AccessibleAbstractTableView.h"

AccessibleAbstractTableViewCellTitle::AccessibleAbstractTableViewCellTitle(AccessibleAbstractTableView* parent, int column)
    : AccessibleAbstractTableViewCell(parent, -1, column)
{
}

QString AccessibleAbstractTableViewCellTitle::text(QAccessible::Text t) const
{
    if(!isValid())
        return QString();
    if(t == QAccessible::Name)
        return mParent->getTable()->getColTitle(mParent->logicalColumn(column));
    return QString();
}

QColor AccessibleAbstractTableViewCellTitle::foregroundColor() const
{
    return isValid() ? mParent->getTable()->mHeaderTextColor : QColor();
}

QColor AccessibleAbstractTableViewCellTitle::backgroundColor() const
{
    return isValid() ? mParent->getTable()->mHeaderBackgroundColor : QColor();
}

QAccessible::State AccessibleAbstractTableViewCellTitle::state() const
{
    QAccessible::State result;
    if(!isValid())
    {
        result.invalid = true;
        return result;
    }

    const AbstractTableView* table = mParent->getTable();
    const bool visible = !rect().isEmpty();
    result.disabled = !table->isEnabled();
    result.readOnly = true;
    result.invisible = table->getHeaderHeight() == 0 || !table->isVisible() || !visible;
    result.offscreen = !visible;
    return result;
}

QRect AccessibleAbstractTableViewCellTitle::rect() const
{
    return mParent ? mParent->elementRect(-1, column, true) : QRect();
}

QAccessible::Role AccessibleAbstractTableViewCellTitle::role() const
{
    return QAccessible::ColumnHeader;
}

int AccessibleAbstractTableViewCellTitle::rowIndex() const
{
    return -1;
}

bool AccessibleAbstractTableViewCellTitle::isValid() const
{
    return mParent && mParent->headerIsValid(column);
}

void* AccessibleAbstractTableViewCellTitle::interface_cast(QAccessible::InterfaceType type)
{
    Q_UNUSED(type);
    // Headers are separate structural children, not cells in the data grid.
    return nullptr;
}

QList<QAccessibleInterface*> AccessibleAbstractTableViewCellTitle::rowHeaderCells() const
{
    return QList<QAccessibleInterface*>();
}

QList<QAccessibleInterface*> AccessibleAbstractTableViewCellTitle::columnHeaderCells() const
{
    return QList<QAccessibleInterface*>({const_cast<AccessibleAbstractTableViewCellTitle*>(this)});
}

#endif
