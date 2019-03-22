#include "itemgroup.h"

#include <mainwindow.h>

ItemGroup::~ItemGroup()
{
    //qDebug("~ItemGroup()");
    if (Scene::self)
        qDeleteAll(*this);
}

void ItemGroup::append(GraphicsItem* value)
{
    value->setItemGroup(this);
    QList::append(value);
}

void ItemGroup::setVisible(const bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;
        for (QGraphicsItem* item : *this)
            item->setVisible(m_visible);
    }
}

void ItemGroup::addToTheScene()
{
    if (Scene::self)
        for (QGraphicsItem* item : *this)
            Scene::self->addItem(item);
}

void ItemGroup::setBrush(const QBrush& brush)
{
    if (m_brush != brush) {
        m_brush = brush;
        for (GraphicsItem* item : *this)
            item->setBrush(m_brush);
    }
}

void ItemGroup::setPen(const QPen& pen)
{
    if (m_pen != pen) {
        m_pen = pen;
        for (GraphicsItem* item : *this)
            item->setPen(m_pen);
    }
}

void ItemGroup::setZValue(qreal z)
{
    for (GraphicsItem* item : *this)
        item->setZValue(z);
}
