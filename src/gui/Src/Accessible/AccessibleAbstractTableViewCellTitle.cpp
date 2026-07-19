// This file implements accessibility interface for the column headers of AbstractTableView
#ifndef QT_NO_ACCESSIBILITY
#include "AccessibleAbstractTableViewCellTitle.h"
#include "AccessibleAbstractTableView.h"

AccessibleAbstractTableViewCellTitle::AccessibleAbstractTableViewCellTitle(AbstractTableView* tableView, int column, quint64 modelRevision)
    : AccessibleAbstractTableViewCell(tableView, -1, column, modelRevision)
{
}

QString AccessibleAbstractTableViewCellTitle::text(QAccessible::Text t) const
{
    AccessibleAbstractTableView* accessible = accessibleTable();
    if(!belongsTo(accessible) || !accessible->headerIsValid(column))
        return QString();
    if(t == QAccessible::Name)
        return mTableView->getColTitle(accessible->logicalColumn(column));
    return QString();
}

QColor AccessibleAbstractTableViewCellTitle::foregroundColor() const
{
    AccessibleAbstractTableView* accessible = accessibleTable();
    return belongsTo(accessible) && accessible->headerIsValid(column) && mTableView
           ? mTableView->mHeaderTextColor
           : QColor();
}

QColor AccessibleAbstractTableViewCellTitle::backgroundColor() const
{
    AccessibleAbstractTableView* accessible = accessibleTable();
    return belongsTo(accessible) && accessible->headerIsValid(column) && mTableView
           ? mTableView->mHeaderBackgroundColor
           : QColor();
}

QAccessible::State AccessibleAbstractTableViewCellTitle::state() const
{
    QAccessible::State result;
    AccessibleAbstractTableView* accessible = accessibleTable();
    const AbstractTableView* table = mTableView.data();
    if(!belongsTo(accessible) || !accessible->headerIsValid(column) || !table)
    {
        result.invalid = true;
        return result;
    }

    const bool visible = !rect().isEmpty();
    result.disabled = !table->isEnabled();
    result.readOnly = true;
    result.invisible = table->getHeaderHeight() == 0 || !table->isVisible() || !visible;
    result.offscreen = !visible;
    return result;
}

QRect AccessibleAbstractTableViewCellTitle::rect() const
{
    AccessibleAbstractTableView* accessible = accessibleTable();
    return belongsTo(accessible) ? accessible->elementRect(-1, column, true) : QRect();
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
    AccessibleAbstractTableView* accessible = accessibleTable();
    return belongsTo(accessible) && accessible->headerIsValid(column);
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
