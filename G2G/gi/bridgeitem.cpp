#include "bridgeitem.h"
#include "itemgroup.h"

#include <QApplication>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <gcode/toolpathcreator.h>
#include <limits>
#include <graphicsview.h>
#include <scene.h>

BridgeItem::BridgeItem(double& lenght, BridgeItem*& ptr, double& size)
    : m_ptr(ptr)
    , m_lenght(lenght)
    , m_size(size)
{
    m_path.addEllipse(QPointF(), m_lenght / 2, m_lenght / 2);
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);
}

QRectF BridgeItem::boundingRect() const
{
    return m_path.boundingRect();
}

void BridgeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setBrush(/*scene()->collidingItems(this).isEmpty()*/ !m_ok ? Qt::red : Qt::green);
    painter->setPen(Qt::NoPen);
    painter->drawPath(m_path);
    painter->setBrush(Qt::magenta);

    //    QLineF l(0, 0, m_lenght / 2 + m_size / 2, 0);
    //    l.setAngle(m_angle + 90);
    //    painter->drawEllipse(l.p2(), m_size / 2, m_size / 2);
    //    l.setAngle(m_angle - 90);
    //    painter->drawEllipse(l.p2(), m_size / 2, m_size / 2);

    //    QLineF l2(0, 0, m_size / 2, 0);
    //    l2.setAngle(m_angle);
    //    painter->drawEllipse(l2.p2(), m_size / 2, m_size / 2);
    //    l2.setAngle(m_angle + 180);
    //    painter->drawEllipse(l2.p2(), m_size / 2, m_size / 2);
}

void BridgeItem::setNewPos(const QPointF& pos) { setPos(pos); }

QVariant BridgeItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionChange /*&& scene()*/) {
        return calculate(value.toPointF());
    } else
        return QGraphicsItem::itemChange(change, value);

    //    if (change == ItemPositionChange) {
    //        //        return QGraphicsObject::itemChange(change, value);   QPointF newPos = value.toPointF();
    //        if (QApplication::mouseButtons() == Qt::LeftButton && qobject_cast<Scene*>(scene())) {
    //            Scene* customScene = qobject_cast<Scene*>(scene());
    //            int gridSize = customScene->getGridSize();
    //            qreal xV = round(newPos.x() / gridSize) * gridSize;
    //            qreal yV = round(newPos.y() / gridSize) * gridSize;
    //            return QPointF(xV, yV);
    //        } else
    //            return newPos;
    //    } else
    //        return QGraphicsObject::itemChange(change, value);
}

void BridgeItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (!m_ok)
        deleteLater();
    disconnect(GraphicsView::self, &GraphicsView::mouseMove, this, &BridgeItem::setNewPos);
    QGraphicsItem::mousePressEvent(event);
}

QPointF BridgeItem::calculate(const QPointF& pos)
{
    QList<QGraphicsItem*> col(scene()->collidingItems(this));
    if (col.isEmpty())
        return pos;

    QPointF pt;
    double l = std::numeric_limits<double>::max();
    double lastAngle = 0.0;
    for (QGraphicsItem* item : col) {
        GraphicsItem* gi = dynamic_cast<GraphicsItem*>(item);
        if (gi && (gi->type() == DrillItemType || gi->type() == GerberItemType || gi->type() == RawItemType)) {
            //if (!gi->isSelected())
            //  continue;
            for (const Path& path : gi->paths()) {
                for (int i = 0, s = path.size(); i < s; ++i) {
                    const QLineF l1(pos, toQPointF(path[i]));
                    const QLineF l2(pos, toQPointF(path[(i + 1) % s]));
                    const QLineF l3(toQPointF(path[(i + 1) % s]), toQPointF(path[i]));
                    if (lastAngle == 0.0)
                        lastAngle = l3.normalVector().angle();
                    const double p = (l1.length() + l2.length() + l3.length()) / 2;
                    if (l1.length() < l3.length() && l2.length() < l3.length()) {
                        const double h = (2 / l3.length()) * sqrt(p * (p - l1.length()) * (p - l2.length()) * (p - l3.length()));
                        if (l > h) {
                            l = h;
                            QLineF line(toQPointF(path[i]), toQPointF(path[(i + 1) % s]));
                            line.setLength(sqrt(l1.length() * l1.length() - h * h));
                            pt = line.p2();
                            m_angle = line.normalVector().angle();
                        }
                    } /*else if (l1.length() < l) {
                        l = l1.length();
                        pt = toQPointF(path[i]);
                        m_angle = l3.normalVector().angle();
                        if (m_angle < lastAngle)
                            m_angle = m_angle + (m_angle - lastAngle) / 2;
                        else
                            m_angle = m_angle + (lastAngle - m_angle) / 2;
                    } else if (l2.length() < l) {
                        l = l2.length();
                        pt = toQPointF(path[(i + 1) % s]);
                        m_angle = l3.normalVector().angle();
                        if (m_angle < lastAngle)
                            m_angle = m_angle + (lastAngle - m_angle) / 2;
                        else
                            m_angle = m_angle + (m_angle - lastAngle) / 2;
                    }*/
                    lastAngle = l3.normalVector().angle();
                }
            }
        }
    }
    if (l < m_lenght / 2) {
        m_ok = true;
        return pt;
    }
    m_ok = false;
    return pos;
}

double BridgeItem::angle() const
{
    return m_angle;
}

void BridgeItem::update()
{
    m_path = QPainterPath();
    m_path.addEllipse(QPointF(), m_lenght / 2, m_lenght / 2);
    QGraphicsItem::update();
}

IntPoint BridgeItem::getPoint(int side)
{
    QLineF l2(0, 0, m_size / 2, 0);
    l2.translate(pos());
    switch (side) {
    case On:
        return toIntPoint(pos());
    case Outer:
        l2.setAngle(m_angle + 180);
        return toIntPoint(l2.p2());
    case Inner:
        l2.setAngle(m_angle);
        return toIntPoint(l2.p2());
    }
    return IntPoint();
}

double BridgeItem::lenght() const
{
    return m_lenght;
}

bool BridgeItem::ok() const
{
    return m_ok;
}

QPainterPath BridgeItem::shape() const
{
    return m_path;
}

void BridgeItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    //setPos(calculate(mapToScene(event->pos())));
    QGraphicsItem::mouseMoveEvent(event);
}

void BridgeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* /*event*/)
{
    deleteLater();
}

void BridgeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (!m_ok)
        deleteLater();
    QGraphicsItem::mouseReleaseEvent(event);
}

Paths BridgeItem::paths() const
{
    return Paths();
}

int BridgeItem::type() const
{
    return BridgeType;
}
