#include "treeitem.h"
#include <QStringList>

TreeItem::TreeItem(const Tool& data, TreeItem* parent)
{
    parentItem = parent;
    itemData = data;
}

TreeItem::~TreeItem()
{
    qDeleteAll(childItems);
}

TreeItem* TreeItem::child(int number)
{
    return childItems.value(number);
}

int TreeItem::childCount() const
{
    return childItems.count();
}

int TreeItem::childNumber() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));
    return 0;
}

int TreeItem::columnCount() const
{
    return 4; //itemData.count();
}

QVariant TreeItem::data(int column) const
{
    switch (column) {
    case 0:
        return itemData.name;
    case 1:
        return itemData.note;
    case 2:
        return itemData.data.toolType;
    case 3:
        return itemData.toHex();
    case Qt::UserRole:
        return QVariant::fromValue(itemData.data.toolType);
    default:
        break;
    }
    return QVariant::fromValue(itemData);
}

bool TreeItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > childItems.size())
        return false;

    for (int row = 0; row < count; ++row) {
        TreeItem* item = new TreeItem(Tool(), this);
        childItems.insert(position, item);
    }

    return true;
}

bool TreeItem::insertColumns(int position, int columns)
{
    //    if (position < 0 || position > itemData.size())
    //        return false;

    //    for (int column = 0; column < columns; ++column)
    //        itemData.insert(position, QVariant());

    //    for (TreeItem* child : childItems)
    //        child->insertColumns(position, columns);

    return true;
}

TreeItem* TreeItem::parent() const
{
    return parentItem;
}

bool TreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);

    return true;
}

bool TreeItem::removeColumns(int position, int columns)
{
    //    if (position < 0 || position + columns > itemData.size())
    //        return false;
    //    for (int column = 0; column < columns; ++column)
    //        itemData.remove(position);
    //    for (TreeItem* child : childItems)
    //        child->removeColumns(position, columns);
    return true;
}

bool TreeItem::setData(const Tool& value)
{
    //    if (column < 0 || column >= itemData.size())
    //        return false;
    itemData = value;
    return true;
}

Tool& TreeItem::getItemData()
{
    return itemData;
}

QList<TreeItem*> TreeItem::getChildItems() const
{
    return childItems;
}
