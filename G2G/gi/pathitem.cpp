#include "pathitem.h"

#include <QPainter>
#include <mygraphicsview.h>

PathItem::PathItem(const Paths& paths)
{
    m_paths = paths;
    for (const Path& path : m_paths)
        m_shape.addPolygon(toQPolygon(path));

    const double k = m_pen.widthF() / 2;
    m_rect = m_shape.boundingRect() + QMarginsF(k, k, k, k);
    //    setAcceptTouchEvents(false);
    //    setAcceptedMouseButtons(false);
    //    setAcceptHoverEvents(false);
    //    setEnabled(false);
    //    setFlags(GraphicsItemFlags(0));
    //    setCacheMode(DeviceCoordinateCache);
}

QRectF PathItem::boundingRect() const { return m_rect; }

void PathItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{

    //    if (0) {
    //        painter->setBrush(QBrush(Qt::NoBrush));
    //        painter->setPen(m_pen);
    //        painter->setPen(QPen(Qt::white, 0.0));
    //        painter->drawPath(m_shape);
    //        painter->setBrush(QBrush(Qt::red));
    //        const double d = m_pen.widthF();
    //        QRectF r(m_paths[0].first().X * dScale - d / 2, m_paths[0].first().Y * dScale - d / 2, d, d);
    //        painter->drawEllipse(r);
    //    } else {

    if (m_penColor)
        m_pen.setColor(*m_penColor);
    if (m_brushColor)
        m_brush.setColor(*m_brushColor);

    if (m_pen.widthF() == 0) {
        QPen pen(m_pen);
        pen.setWidthF(1.5 / MyGraphicsView::self->matrix().m11());
        painter->setPen(pen);
    } else
        painter->setPen(m_pen);

    painter->setBrush(QBrush(Qt::NoBrush));
    painter->drawPath(m_shape);

    ////////////////////////////////////////////////////// for debug cut direction

    //    if (m_pen.widthF() == 0) {
    //        painter->setPen(QPen(Qt::green, 0.0));
    //        for (Path m_path : m_paths) {
    //            //                QLineF line(toQPointF(m_path[0]), toQPointF(m_path[1]));
    //            //                double length = 50 / MyGraphicsView::self->matrix().m11();
    //            //                double angle = line.angle();
    //            //                line.setLength(length);
    //            //                painter->drawLine(line);
    //            //                line.setAngle(angle + 10);
    //            //                painter->drawLine(line);
    //            //                line.setAngle(angle - 10);
    //            //                painter->drawLine(line);
    //            for (int i = 0; i < m_path.size() - 1; ++i) {
    //                QLineF line(toQPointF(m_path[i]), toQPointF(m_path[i + 1]));

    //                double length = 50 / MyGraphicsView::self->matrix().m11();
    //                double angle = line.angle();

    //                line.setLength(length);
    //                //                    painter->drawLine(line);

    //                line.setAngle(angle + 10);
    //                painter->drawLine(line);

    //                line.setAngle(angle - 10);
    //                painter->drawLine(line);
    //            }
    //        }
    //    }
    //}
}

int PathItem::type() const { return PathItemType; }

Paths PathItem::paths() const { return m_paths; }
