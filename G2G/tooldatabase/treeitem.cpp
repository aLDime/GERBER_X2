#include "treeitem.h"

int ToolItem::c = 0;

ToolItem::ToolItem(const ToolItem& item)
{
    if (this == &item)
        return;
    tool = item.tool;
    for (const ToolItem* i : item.childItems) {
        addChild(new ToolItem(*i));
    }
}

ToolItem::ToolItem(const Tool& tool)
    : tool(tool)
{
    qDebug() << "+TreeItem" << ++c;
}

ToolItem::~ToolItem()
{
    qDebug() << "~TreeItem" << c--;
    qDeleteAll(childItems);
}

int ToolItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<ToolItem*>(this));
    return 0;
}

int ToolItem::childCount() const { return childItems.size(); }

ToolItem* ToolItem::child(int row) { return childItems.at(row); }

void ToolItem::setChild(int row, ToolItem* item)
{
    if (item)
        item->parentItem = this;

    if (row < childItems.size()) {
        if (childItems[row])
            delete childItems[row];
        childItems[row] = item;
    }
}

void ToolItem::addChild(ToolItem* item)
{
    if (item)
        item->parentItem = this;
    childItems.append(item);
}

void ToolItem::insertChild(int row, ToolItem* item)
{
    if (item)
        item->parentItem = this;
    if (row < childItems.size())
        childItems.insert(row, item);
    else if (row == childItems.size())
        childItems.append(item);
}

void ToolItem::removeChild(int row)
{
    delete childItems.at(row);
    childItems.removeAt(row);
}

ToolItem* ToolItem::parent() { return parentItem; }
