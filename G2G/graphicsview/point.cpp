#include "point.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDebug>
#include <materialsetup.h>

Point::Point(int type)
    : pen(Qt::NoPen)
    , type(type)
{
    setToolTip("G-Code Home Point");
    setFlags(QGraphicsItem::ItemIsMovable);

    if (type) {
        path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 0, 90);
        path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 270, -90);
    }
    else {
        path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 90, 90);
        path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 360, -90);
    }
    shape_.addEllipse(QRectF(QPointF(-3, -3), QSizeF(6, 6)));
    rect = path.boundingRect();
}

QRectF Point::boundingRect() const
{
    return rect;
}

void Point::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);

    painter->setPen(Qt::NoPen);
    painter->setBrush(brush);
    painter->drawPath(path);

    painter->setPen(QPen(brush.color(), 1.0));
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPoint(0, 0), 2, 2);
}

QPainterPath Point::shape() const
{
    return shape_;
}

void Point::setBrush(const QBrush& value)
{
    brush = value;
}

void Point::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseMoveEvent(event);
    if (type)
        MaterialSetup::This()->setHomePos(pos());
    else
        MaterialSetup::This()->setZeroPos(pos());
}

void Point::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    setPos(QPointF());
    QGraphicsItem::mouseDoubleClickEvent(event);
    if (type)
        MaterialSetup::This()->setHomePos(pos());
    else
        MaterialSetup::This()->setZeroPos(pos());
}
