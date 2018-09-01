#ifndef TREEITEM_H
#define TREEITEM_H

#include <QAbstractItemModel>
#include <QVariant>

class AbstractItem {
    AbstractItem(const AbstractItem&) = delete;
    AbstractItem& operator=(const AbstractItem&) = delete;
    static int c;

public:
    AbstractItem();
    virtual ~AbstractItem();
    AbstractItem* child(int row);
    AbstractItem* parentItem();

    int row() const;

    void add(AbstractItem* item);
    void insert(int row, AbstractItem* item);
    void remove(int row);
    virtual int childCount() const;

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) = 0;
    virtual int columnCount() const = 0;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const = 0;
    virtual QVariant data(const QModelIndex& index, int role) const = 0;

protected:
    AbstractItem* m_parentItem = nullptr;
    QList<AbstractItem*> childItems;
};

#endif // TREEITEM_H
