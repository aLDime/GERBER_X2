#ifndef TREEITEM_H
#define TREEITEM_H

#include "tool.h"

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

    bool setData(const QModelIndex& index, const QVariant& value, int role);

    QVariant data(const QModelIndex& index, int role) const;

    Qt::ItemFlags flags(const QModelIndex& /*index*/) const;

    Tool getTool() const;
    void setTool(const Tool &value);

private:
    ToolItem* parentItem = nullptr;
    QList<ToolItem*> childItems;
    Tool tool;
};
#endif // TREEITEM_H
