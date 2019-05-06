#include "rawitem.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <gbrfile.h>
#include <graphicsview.h>
#include <myclipper.h>

#include "forms/materialsetupform.h"

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
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
}

QRectF RawItem::boundingRect() const { return m_shape.boundingRect(); }

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
        pen.setWidthF(2.0 / GraphicsView::self->matrix().m11());
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

QPainterPath RawItem::shape() const
{
    if (m_scale != GraphicsView::self->matrix().m11()) {
        m_scale = GraphicsView::self->matrix().m11();
        m_shape = QPainterPath();
        ClipperOffset offset;
        Paths tmpPpath;
        offset.AddPath(m_path, jtSquare, etOpenButt);
        offset.Execute(tmpPpath, 6 * uScale / m_scale);
        for (const Path& path : tmpPpath)
            m_shape.addPolygon(toQPolygon(path));
    }
    return m_shape;
}

const Gerber::File* RawItem::file() const { return m_file; }

void RawItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    event->ignore();
    if (event->modifiers() & Qt::ShiftModifier) {
        const double glueLen = MaterialSetup::glue * uScale;
        IntPoint dest(m_path.last());
        IntPoint init(m_path.last());
        QList<int> skip;
        ItemGroup* ig = m_file->rawItemGroup();

        for (int i = 0; i < ig->size(); ++i) {
            if (skip.contains(i))
                continue;
            const Path path(ig->at(i)->paths().first());
            const IntPoint& first = path.first();
            const IntPoint& last = path.last();
            if (Length(dest, first) < glueLen) {
                dest = last;
                skip.append(i);
                ig->at(i)->setSelected(true);
                if (Length(init, dest) < glueLen) //init == dest)
                    break;
                i = -1;
            } else if (Length(dest, last) < glueLen) {
                dest = first;
                skip.append(i);
                ig->at(i)->setSelected(true);
                if (Length(init, dest) < glueLen) //init == dest)
                    break;
                i = -1;
            }
        }
        return;
    }
    GraphicsItem::mouseReleaseEvent(event);
}
