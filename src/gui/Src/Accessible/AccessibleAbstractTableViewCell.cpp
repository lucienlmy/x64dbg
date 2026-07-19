// This file implements accessibility interface for table cells of AbstractTableView
#ifndef QT_NO_ACCESSIBILITY
#include "AccessibleAbstractTableViewCell.h"
#include "AccessibleAbstractTableView.h"
#include <exception>

AccessibleAbstractTableViewCell::AccessibleAbstractTableViewCell(AccessibleAbstractTableView* parent, int row, int column)
    : row(row)
    , column(column)
    , mParent(parent)
{
}

QString AccessibleAbstractTableViewCell::text(QAccessible::Text t) const
{
    if(!isValid())
        return QString();
    switch(t)
    {
    case QAccessible::Name:
        try
        {
            return mParent->getCellContent(row, mParent->logicalColumn(column));
        }
        catch(const std::exception &)
        {
            // Accessibility APIs must reject stale virtual cells, not propagate
            // an out-of-range exception into a native platform bridge.
            return QString();
        }
    default:
        return QString();
    }
}

QColor AccessibleAbstractTableViewCell::foregroundColor() const
{
    return isValid() ? mParent->getTable()->mTextColor : QColor();
}

int AccessibleAbstractTableViewCell::childCount() const
{
    return 0;
}

QWindow* AccessibleAbstractTableViewCell::window() const
{
    return mParent ? mParent->window() : nullptr;
}

QAccessibleInterface* AccessibleAbstractTableViewCell::parent() const
{
    return mParent ? static_cast<QAccessibleInterface*>(mParent) : nullptr;
}

QAccessibleInterface* AccessibleAbstractTableViewCell::child(int index) const
{
    Q_UNUSED(index);
    return nullptr;
}

int AccessibleAbstractTableViewCell::indexOfChild(const QAccessibleInterface* child) const
{
    Q_UNUSED(child);
    return -1;
}

QAccessible::Role AccessibleAbstractTableViewCell::role() const
{
    return QAccessible::Cell;
}

QAccessible::State AccessibleAbstractTableViewCell::state() const
{
    QAccessible::State result;
    if(!isValid())
    {
        result.invalid = true;
        return result;
    }

    const AbstractTableView* table = mParent->getTable();
    const bool enabled = table->isEnabled();
    const bool visible = !rect().isEmpty();
    result.disabled = !enabled;
    result.focusable = enabled;
    result.selectable = enabled;
    result.selected = isSelected();
    result.focused = result.selected
                     && table->accessibilitySelectedColumn == column
                     && table->hasFocus();
    result.readOnly = true;
    result.invisible = !table->isVisible() || !visible;
    result.offscreen = !visible;
    return result;
}

QAccessibleInterface* AccessibleAbstractTableViewCell::childAt(int x, int y) const
{
    Q_UNUSED(x);
    Q_UNUSED(y);
    return nullptr;
}

QObject* AccessibleAbstractTableViewCell::object() const
{
    return nullptr;
}

void AccessibleAbstractTableViewCell::setText(QAccessible::Text t, const QString & text)
{
    Q_UNUSED(t);
    Q_UNUSED(text);
}

QRect AccessibleAbstractTableViewCell::rect() const
{
    return mParent ? mParent->elementRect(row, column, false) : QRect();
}

bool AccessibleAbstractTableViewCell::isValid() const
{
    return mParent && mParent->cellIsValid(row, column);
}

void* AccessibleAbstractTableViewCell::interface_cast(QAccessible::InterfaceType type)
{
    if(type == QAccessible::TableCellInterface)
        return static_cast<QAccessibleTableCellInterface*>(this);
    return nullptr;
}

bool AccessibleAbstractTableViewCell::isSelected() const
{
    return isValid() && mParent->isRowSelected(row);
}

QList<QAccessibleInterface*> AccessibleAbstractTableViewCell::columnHeaderCells() const
{
    if(!isValid())
        return QList<QAccessibleInterface*>();
    if(auto header = mParent->columnHeaderInterface(column))
        return QList<QAccessibleInterface*>({header});
    return QList<QAccessibleInterface*>();
}

QList<QAccessibleInterface*> AccessibleAbstractTableViewCell::rowHeaderCells() const
{
    // AbstractTableView does not paint or expose row headers.
    return QList<QAccessibleInterface*>();
}

int AccessibleAbstractTableViewCell::columnIndex() const
{
    return column;
}

int AccessibleAbstractTableViewCell::rowIndex() const
{
    return row;
}

int AccessibleAbstractTableViewCell::columnExtent() const
{
    return 1;
}

int AccessibleAbstractTableViewCell::rowExtent() const
{
    return 1;
}

QAccessibleInterface* AccessibleAbstractTableViewCell::table() const
{
    return mParent ? static_cast<QAccessibleInterface*>(mParent) : nullptr;
}

#endif
