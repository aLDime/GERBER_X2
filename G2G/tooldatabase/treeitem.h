#ifndef TREEITEM_H
#define TREEITEM_H

#include "edittool.h"

#include <QObject>
#include <QVariant>
#include <QDebug>
#include <QAbstractItemModel>

class ToolItem {
    //TreeItem(const TreeItem&) Q_DECL_EQ_DELETE;
    ToolItem& operator=(const ToolItem&) Q_DECL_EQ_DELETE;
    static int c;

public:
    ToolItem(const Tool& tool);
    ToolItem(const ToolItem& item);
    ~ToolItem();
    int row() const;
    int childCount() const;
    ToolItem* child(int row);
    ToolItem* parent();
    void addChild(ToolItem* item);
    void insertChild(int row, ToolItem* item);
    void removeChild(int row);
    void setChild(int row, ToolItem* item);

    bool setData(const QModelIndex& index, const QVariant& value, int role)
    {
        return true;
    }

    QVariant data(const QModelIndex& index, int role) const
    {
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
            default:
                return QVariant();
            }
        }
        return QVariant();
    }

    Qt::ItemFlags flags(const QModelIndex& /*index*/) const
    {
        Qt::ItemFlags defaultFlags = Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable;
        if (tool.data.toolType == ToolType::Group)
            defaultFlags |= Qt::ItemIsDropEnabled;
        return defaultFlags;
        //        if (index.column())
        //            return defaultFlags;
        //        if (index.isValid())
        //            return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
        //        else
        //            return Qt::ItemIsDropEnabled | defaultFlags;
    }

private:
    ToolItem* parentItem = nullptr;
    QList<ToolItem*> childItems;
    Tool tool;
};
#endif // TREEITEM_H
