#include "abstractnode.h"

#include <QDebug>

int AbstractNode::c = 0;
QList<QString> AbstractNode::files;

AbstractNode::AbstractNode()
{
    //qDebug() << "AbstractItem" << ++c << this << m_parentItem;
}

AbstractNode::~AbstractNode()
{
    //qDebug() << "~AbstractItem" << c-- << this << m_parentItem;
    qDeleteAll(childItems);
}

int AbstractNode::row() const
{
    if (m_parentItem)
        return m_parentItem->childItems.indexOf(const_cast<AbstractNode*>(this));
    return 0;
}

AbstractNode* AbstractNode::child(int row)
{
    return childItems.value(row);
}

AbstractNode* AbstractNode::parentItem()
{
    return m_parentItem;
}

void AbstractNode::add(AbstractNode* item)
{
    item->m_parentItem = this;
    childItems.append(item);
}

void AbstractNode::insert(int row, AbstractNode* item)
{
    item->m_parentItem = this;
    if (row < childItems.size())
        childItems.insert(row, item);
    else if (row == childItems.size())
        childItems.append(item);
}

void AbstractNode::remove(int row)
{
    delete childItems.takeAt(row);
}

int AbstractNode::childCount() const
{
    return childItems.count();
}
