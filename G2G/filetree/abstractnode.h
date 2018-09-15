#ifndef TREEITEM_H
#define TREEITEM_H

#include <QAbstractItemModel>
#include <QVariant>

class AbstractNode {
    AbstractNode(const AbstractNode&) = delete;
    AbstractNode& operator=(const AbstractNode&) = delete;
    static int c;

public:
    AbstractNode();
    virtual ~AbstractNode();
    AbstractNode* child(int row);
    AbstractNode* parentItem();

    int row() const;

    void add(AbstractNode* item);
    void insert(int row, AbstractNode* item);
    void remove(int row);
    virtual int childCount() const;

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) = 0;
    virtual int columnCount() const = 0;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const = 0;
    virtual QVariant data(const QModelIndex& index, int role) const = 0;
    static QList<QString> files;

protected:
    AbstractNode* m_parentItem = nullptr;
    QList<AbstractNode*> childItems;
};

#endif // TREEITEM_H
