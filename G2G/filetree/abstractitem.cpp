#include "abstractitem.h"
#include "abstractitem.h"

#include <QIcon>
#include <QPixmap>

int AbstractItem::c = 0;

AbstractItem::AbstractItem()
{
    qDebug() << "~AbstractItem" << ++c;
}

AbstractItem::~AbstractItem()
{
    qDebug() << "~AbstractItem" << c--;
    qDeleteAll(childItems);
}

int AbstractItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<AbstractItem*>(this));
    return 0;
}

AbstractItem* AbstractItem::child(int row)
{
    return childItems.at(row);
}

AbstractItem* AbstractItem::parent()
{
    return parentItem;
}

void AbstractItem::add(AbstractItem* item)
{
    if (item)
        item->parentItem = this;
    childItems.append(item);
}

void AbstractItem::insert(int row, AbstractItem* item)
{
    if (item)
        item->parentItem = this;
    if (row < childItems.size())
        childItems.insert(row, item);
    else if (row == childItems.size())
        childItems.append(item);
}

void AbstractItem::remove(int row)
{
    delete childItems.at(row);
    childItems.removeAt(row);
}

void AbstractItem::set(int row, AbstractItem* item)
{
    if (item)
        item->parentItem = this;
    if (row < childItems.size()) {
        if (childItems[row])
            delete childItems[row];
        childItems[row] = item;
    }
}
