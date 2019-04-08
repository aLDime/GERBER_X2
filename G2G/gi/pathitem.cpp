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
#ifdef QT_DEBUG
    //setAcceptHoverEvents(true);
#endif
}

QRectF PathItem::boundingRect() const { return m_rect; }

void PathItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    Q_UNUSED(option)

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
#ifdef QT_DEBUG
    if (option->state & QStyle::State_MouseOver) {
        QPen pen(m_pen);
        pen.setWidthF(2.0 / GraphicsView::self->matrix().m11());
        pen.setStyle(Qt::CustomDashLine);
        pen.setCapStyle(Qt::FlatCap);
        pen.setDashPattern({ 2.0, 2.0 });
        painter->setPen(pen);
    }
#endif
    painter->setBrush(QBrush(Qt::NoBrush));
    painter->drawPath(m_shape);

    ////////////////////////////////////////////////////// for debug cut direction
#ifdef QT_DEBUG
    if (m_pen.widthF() == 0) {
        for (Path path : m_paths) {
            for (int i = 0; i < path.size() - 1; ++i) {
                QLineF line(toQPointF(path[i + 1]), toQPointF(path[i]));
                double length = 20 / GraphicsView::self->matrix().m11();
                if (line.length() < length && i)
                    continue;
                if (length > 0.5)
                    length = 0.5;
                const double angle = line.angle();
                line.setLength(length);
                line.setAngle(angle + 10);
                painter->drawLine(line);
                line.setAngle(angle - 10);
                painter->drawLine(line);

                if (0) {
                    painter->save();
                    const QString text = "   " + QString::number(i);
                    const QRectF textRect = QFontMetricsF(painter->font()).boundingRect(QRectF(), Qt::AlignLeft, text);
                    const double k = 1.0 / GraphicsView ::self->matrix().m11();
                    painter->translate(toQPointF(path[i]));
                    painter->scale(k, -k);
                    //painter->setBrush(QColor(127, 127, 127, 255));
                    //painter->drawRect(textRect);
                    painter->drawText(textRect, Qt::AlignLeft, text);
                    painter->restore();
                }
            }
        }
    }
#endif
}

int PathItem::type() const { return PathItemType; }

Paths PathItem::paths() const { return m_paths; }
