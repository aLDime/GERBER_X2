#include "point.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDebug>

#ifdef G2G
#include "forms/materialsetup.h"
#endif
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
    if (reinterpret_cast<MyGraphicsScene*>(scene())->drawPdf)
        return QRectF();

    return rect;
}

void Point::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    if (reinterpret_cast<MyGraphicsScene*>(scene())->drawPdf)
        return;

    painter->setPen(Qt::NoPen);
    painter->setBrush(brush);
    painter->drawPath(path);
    painter->setPen(QPen(brush.color(), 1.0));
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPoint(0, 0), 2, 2);
}

QPainterPath Point::shape() const
{
    if (reinterpret_cast<MyGraphicsScene*>(scene())->drawPdf)
        return QPainterPath();

    return shape_;
}

void Point::setBrush(const QBrush& value)
{
    brush = value;
}

void Point::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseMoveEvent(event);
#ifdef G2G
    if (type)
        MaterialSetup::This()->setHomePos(pos());
    else
        MaterialSetup::This()->setZeroPos(pos());
#endif
}

void Point::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    setPos(QPointF());
    QGraphicsItem::mouseDoubleClickEvent(event);
#ifdef G2G
    if (type)
        MaterialSetup::This()->setHomePos(pos());
    else
        MaterialSetup::This()->setZeroPos(pos());
#endif
}
