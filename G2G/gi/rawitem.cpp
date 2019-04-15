#include "rawitem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <file.h>
#include <graphicsview.h>
#include <myclipper.h>

RawItem::RawItem(const Path& path, Gerber::File* file)
    : m_file(file)
{
    m_paths = { path };
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
    //    static int t = 0;
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
        //        pen.setStyle(Qt::DashLine);
        //        pen.setDashOffset(t++ % 10);
    }
    if (option->state & QStyle::State_MouseOver) {
        //color = color.dark(110);
        //color.setAlpha(100);
        //pen.setColor(color);
        pen.setWidthF(2.0 / GraphicsView::self->matrix().m11());
        pen.setStyle(Qt::CustomDashLine);
        pen.setCapStyle(Qt::FlatCap);
        pen.setDashPattern({ 2.0, 2.0 });
        //        pen.setDashOffset(t++ % 10);
    }

    if (pen.widthF() == 0)
        pen.setWidthF(1.5 / GraphicsView::self->matrix().m11());

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPolyline(toQPolygon(m_paths.first()));

    //painter->drawPath(m_shape);
}

int RawItem::type() const { return RawItemType; }

Paths RawItem::paths() const { return m_paths; }

QPainterPath RawItem::shape() const { return m_shape; }

const Gerber::File* RawItem::file() const { return m_file; }
