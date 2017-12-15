#include "toolitem.h"

#include <QIcon>
#include <QPixmap>

int ToolItem::c = 0;

ToolItem::ToolItem(const ToolItem& item)
{
    qDebug() << "+TreeItem" << ++c;
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

ToolItem* ToolItem::takeChild(int row)
{
    ToolItem* item = childItems.at(row);
    childItems.removeAt(row);
    return item;
}

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

bool ToolItem::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.isValid()) {
        switch (role) {
        case Qt::EditRole:
            switch (index.column()) {
            case 0:
                tool.name = value.toString();
                return true;
            case 1:
                tool.note = value.toString();
                return true;
            default:
                return false;
                break;
            }

        default:
            return false;
        }
    }
    return false;
}

QVariant ToolItem::data(const QModelIndex& index, int role) const
{
    QPixmap p(16, 16);
    p.fill(Qt::red);

    if (index.isValid()) {
        switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            switch (index.column()) {
            case 0:
                return tool.name;
            case 1:
                return tool.note;
            default:
                return QVariant();
                break;
            }
        case Qt::DecorationRole:
            if (index.column() == 0) {
                switch (tool.data.toolType) {
                case Group:
                    return QIcon::fromTheme("folder-sync");
                case EndMill:
                    return QIcon::fromTheme("stroke-cap-round");
                case Engraving:
                    return QIcon::fromTheme("stroke-cap-square");
                case Drill:
                    return QIcon::fromTheme("stroke-cap-butt");
                }
            }
            break;
        case Qt::UserRole:
            return tool.data.toolType;
        default:
            return QVariant();
        }
    }
    return QVariant();
}

Qt::ItemFlags ToolItem::flags(const QModelIndex&) const
{
    Qt::ItemFlags defaultFlags = Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable;
    if (tool.data.toolType == ToolType::Group)
        defaultFlags |= Qt::ItemIsDropEnabled;
    return defaultFlags;
}

Tool ToolItem::getTool() const
{
    return tool;
}

void ToolItem::setTool(const Tool& value)
{
    tool = value;
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
