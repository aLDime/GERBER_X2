#include "abstractitem.h"

#include <QDebug>

int AbstractItem::c = 0;

AbstractItem::AbstractItem()
{
    qDebug() << "AbstractItem" << ++c << this << m_parentItem;
}

AbstractItem::~AbstractItem()
{
    qDebug() << "~AbstractItem" << c-- << this << m_parentItem;
    qDeleteAll(childItems);
}

int AbstractItem::row() const
{
    if (m_parentItem)
        return m_parentItem->childItems.indexOf(const_cast<AbstractItem*>(this));
    return 0;
}

AbstractItem* AbstractItem::child(int row)
{
    return childItems.value(row);
}

AbstractItem* AbstractItem::parentItem()
{
    return m_parentItem;
}

void AbstractItem::add(AbstractItem* item)
{
    item->m_parentItem = this;
    childItems.append(item);
}

void AbstractItem::insert(int row, AbstractItem* item)
{
    item->m_parentItem = this;
    if (row < childItems.size())
        childItems.insert(row, item);
    else if (row == childItems.size())
        childItems.append(item);
}

void AbstractItem::remove(int row)
{
    delete childItems.takeAt(row);
}

int AbstractItem::childCount() const
{
    return childItems.count();
}

