#ifndef TREEITEM_H
#define TREEITEM_H

#include "tool.h"

#include <QDebug>
#include <QModelIndex>
#include <QObject>
#include <QVariant>

class ToolItem {
    ToolItem& operator=(const ToolItem&) Q_DECL_EQ_DELETE;

public:
    ToolItem();
    ToolItem(int toolId);
    ToolItem(const ToolItem& item);
    ~ToolItem();

    //    void read(const QJsonObject& json);
    //    void write(QJsonObject& json);

    int row() const;
    int childCount() const;
    ToolItem* child(int row) const;
    ToolItem* lastChild() const;
    ToolItem* takeChild(int row);
    ToolItem* parent();
    void addChild(ToolItem* item);
    void insertChild(int row, ToolItem* item);
    void removeChild(int row);
    void setChild(int row, ToolItem* item);

    bool setData(const QModelIndex& index, const QVariant& value, int role);

    QVariant data(const QModelIndex& index, int role) const;

    Qt::ItemFlags flags(const QModelIndex&) const;

    int toolId() const;

    Tool& tool();

    bool isTool() const;
    void setIsTool();

    QString name() const;
    void setName(const QString& value);

    QString note() const;
    void setNote(const QString& value);

    static void setDeleteEnable(bool deleteEnable);

private:
    static bool m_deleteEnable;
    ToolItem* parentItem = nullptr;
    QList<ToolItem*> childItems;
    mutable int m_toolId = 0;
    QString m_name;
    QString m_note;
};
#endif // TREEITEM_H
