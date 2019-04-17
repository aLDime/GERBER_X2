#include "rawitem.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <gbrfile.h>
#include <graphicsview.h>
#include <myclipper.h>

RawItem::RawItem(Path& path, Gerber::File* file)
    : m_file(file)
    , m_path(path)
{
    m_polygon = toQPolygon(path);

    Paths tmpPpath;
    ClipperOffset offset;
    offset.AddPath(path, jtSquare, etOpenButt);
    offset.Execute(tmpPpath, 0.01 * uScale);
    for (const Path& path : tmpPpath)
        m_shape.addPolygon(toQPolygon(path));

    m_rect = m_shape.boundingRect();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
}

QRectF RawItem::boundingRect() const
{
    int k = 2.0 / GraphicsView::self->matrix().m11();
    return m_rect + QMarginsF(k, k, k, k);
}

void RawItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)

{
    if (m_penColor)
        m_pen.setColor(*m_penColor);
    if (m_brushColor)
        m_brush.setColor(*m_brushColor);

    QColor color(m_pen.color());
    QPen pen(m_pen);

    if (option->state & QStyle::State_Selected) {
        color.setAlpha(255);
        pen.setColor(color);
        pen.setWidthF(2.0 / GraphicsView::self->matrix().m11());
    }
    if (option->state & QStyle::State_MouseOver) {
        pen.setWidthF(3.0 / GraphicsView::self->matrix().m11());
        pen.setStyle(Qt::CustomDashLine);
        pen.setCapStyle(Qt::FlatCap);
        pen.setDashPattern({ 3.0, 3.0 });
    }

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPolyline(toQPolygon(m_path));
}

int RawItem::type() const { return RawItemType; }

Paths RawItem::paths() const { return { m_path }; }

QPainterPath RawItem::shape() const { return m_shape; }

const Gerber::File* RawItem::file() const { return m_file; }

void RawItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    event->ignore();
    if (event->modifiers() & Qt::ShiftModifier) {
        const double glueLen = 0.2 * uScale;
        IntPoint dest(m_path.last());
        IntPoint init(m_path.last());
        QList<int> skip;
        ItemGroup* ig = m_file->rawItemGroup();
        for (int i = 0; i < ig->size(); ++i) {
            if (skip.contains(i))
                continue;
            const IntPoint& first = ig->at(i)->paths().first().first();
            const IntPoint& last = ig->at(i)->paths().first().last();
            if (Length(dest, first) < glueLen) {
                dest = last;
                skip.append(i);
                ig->at(i)->setSelected(true);
                if (init == dest)
                    break;
                i = -1;
            } else if (Length(dest, last) < glueLen) {
                dest = first;
                skip.append(i);
                ig->at(i)->setSelected(true);
                if (init == dest)
                    break;
                i = -1;
            }
        }
        return;
    }
    GraphicsItem::mouseReleaseEvent(event);
}
