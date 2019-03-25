#include "pathitem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <graphicsview.h>

PathItem::PathItem(const Paths& paths)
{
    m_paths = paths;
    for (const Path& path : m_paths)
        m_shape.addPolygon(toQPolygon(path));

    const double k = m_pen.widthF() / 2;
    m_rect = m_shape.boundingRect() + QMarginsF(k, k, k, k);
}

QRectF PathItem::boundingRect() const { return m_rect; }

void PathItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{

    if (m_penColor)
        m_pen.setColor(*m_penColor);
    if (m_brushColor)
        m_brush.setColor(*m_brushColor);

    if (m_pen.widthF() == 0) {
        QPen pen(m_pen);
        pen.setWidthF(1.5 / GraphicsView::self->matrix().m11());
        painter->setPen(pen);
    } else
        painter->setPen(m_pen);

    painter->setBrush(QBrush(Qt::NoBrush));
    painter->drawPath(m_shape);

    ////////////////////////////////////////////////////// for debug cut direction
#ifdef QT_DEBUG
    if (m_pen.widthF() == 0) {
        for (Path path : m_paths) {
            for (int i = 0; i < path.size() - 1; ++i) {
                QLineF line(toQPointF(path[i]), toQPointF(path[i + 1]));
                double length = 50 / GraphicsView::self->matrix().m11();
                if (line.length() < length && i)
                    continue;
                if (length > 1)
                    length = 1;
                const double angle = line.angle();
                line.setLength(length);
                line.setAngle(angle + 10);
                painter->drawLine(line);
                line.setAngle(angle - 10);
                painter->drawLine(line);
            }
        }
    }
#endif
}

int PathItem::type() const { return PathItemType; }

Paths PathItem::paths() const { return m_paths; }
