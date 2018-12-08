#include "abstractnode.h"

#include <QDebug>

int AbstractNode::c = 0;

AbstractNode::AbstractNode()
{
    qDebug() << "AbstractItem" << ++c << this << m_parentItem;
}

AbstractNode::~AbstractNode()
{
    qDebug() << "~AbstractItem" << c-- << this << m_parentItem;
    //    if (childItems.size())
    childItems.clear();
    //qDeleteAll(childItems);
}

int AbstractNode::row() const
{
    if (m_parentItem)
        for (int i = 0, size = m_parentItem->childItems.size(); i < size; ++i)
            if (m_parentItem->childItems[i].data() == this)
                return i;
    //return m_parentItem->childItems.indexOf(const_cast<AbstractNode*>(this));
    return 0;
}

AbstractNode* AbstractNode::child(int row) { return childItems.value(row).data(); }

AbstractNode* AbstractNode::parentItem() { return m_parentItem; }

void AbstractNode::append(AbstractNode* item)
{
    item->m_parentItem = this;
    childItems.append(QSharedPointer<AbstractNode>(item));
}

void AbstractNode::remove(int row) { childItems.removeAt(row); }

int AbstractNode::childCount() const { return childItems.count(); }
