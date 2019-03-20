#include "bridgeitem.h"
#include "itemgroup.h"

#include <QApplication>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <limits>
#include <mygraphicsview.h>
#include <myscene.h>

BridgeItem::BridgeItem(double lenght, BridgeItem*& ptr)
    : m_ptr(ptr)
    , m_lenght(lenght)
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
    painter->setBrush(scene()->collidingItems(this).isEmpty() ? Qt::red : Qt::green);
    painter->setPen(Qt::NoPen);
    painter->drawPath(m_path);
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
    disconnect(MyGraphicsView::self, &MyGraphicsView::mouseMove, this, &BridgeItem::setNewPos);
    QGraphicsItem::mousePressEvent(event);
}

QPointF BridgeItem::calculate(const QPointF& pos)
{
    QList<QGraphicsItem*> col(scene()->collidingItems(this));
    if (col.isEmpty())
        return pos;

    QPointF pt;
    double l = std::numeric_limits<double>::max();
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
                    } else if (l1.length() < l) {
                        l = l1.length();
                        pt = toQPointF(path[i]);
                        m_angle = l3.normalVector().angle();
                    } else if (l2.length() < l) {
                        l = l2.length();
                        pt = toQPointF(path[(i + 1) % s]);
                        m_angle = l3.normalVector().angle();
                    }
                }
            }
        }
    }
    if (l < m_lenght) {
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
