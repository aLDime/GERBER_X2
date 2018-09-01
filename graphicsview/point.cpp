#include "point.h"

#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#ifdef G2G
#include "forms/materialsetupform.h"
#endif

Point::Point(int type)
    : m_pen(Qt::NoPen)
    , m_type(type)
{
    setToolTip("G-Code Home Point");
    setFlags(QGraphicsItem::ItemIsMovable);

    if (m_type) {
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 0, 90);
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 270, -90);
    } else {
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 90, 90);
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 360, -90);
    }
    m_shape.addEllipse(QRectF(QPointF(-3, -3), QSizeF(6, 6)));
    m_rect = m_path.boundingRect();
}

QRectF Point::boundingRect() const
{
    if (MyScene::self != nullptr && MyScene::self->drawPdf)
        return QRectF();
    return m_rect;
}

void Point::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    if (reinterpret_cast<MyScene*>(scene())->drawPdf)
        return;

    painter->setPen(Qt::NoPen);
    painter->setBrush(m_brush);
    painter->drawPath(m_path);
    painter->setPen(QPen(m_brush.color(), 1.0));
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPoint(0, 0), 2, 2);
}

QPainterPath Point::shape() const
{
    if (MyScene::self != nullptr && MyScene::self->drawPdf)
        return QPainterPath();
    return m_shape;
}

void Point::setBrush(const QBrush& brush)
{
    m_brush = brush;
}

void Point::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseMoveEvent(event);
#ifdef G2G
    if (m_type)
        MaterialSetupForm::self->setHomePos(pos());
    else
        MaterialSetupForm::self->setZeroPos(pos());
#endif
}

void Point::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    setPos(QPointF());
    QGraphicsItem::mouseDoubleClickEvent(event);
#ifdef G2G
    if (m_type)
        MaterialSetupForm::self->setHomePos(pos());
    else
        MaterialSetupForm::self->setZeroPos(pos());
#endif
}
