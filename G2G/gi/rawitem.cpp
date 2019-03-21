#include "rawitem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <file.h>
#include <myclipper.h>
#include <mygraphicsview.h>

RawItem::RawItem(const Path& path, G::File* file)
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
    int k = 2.0 / MyGraphicsView::self->matrix().m11();
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
        pen.setWidthF(2.0 / MyGraphicsView::self->matrix().m11());
        //        pen.setStyle(Qt::DashLine);
        //        pen.setDashOffset(t++ % 10);
    }
    if (option->state & QStyle::State_MouseOver) {
        //color = color.dark(110);
        //color.setAlpha(100);
        //pen.setColor(color);
        pen.setWidthF(2.0 / MyGraphicsView::self->matrix().m11());
        pen.setStyle(Qt::CustomDashLine);
        pen.setCapStyle(Qt::FlatCap);
        pen.setDashPattern({ 2.0, 2.0 });
        //        pen.setDashOffset(t++ % 10);
    }

    if (pen.widthF() == 0)
        pen.setWidthF(1.5 / MyGraphicsView::self->matrix().m11());

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPolyline(toQPolygon(m_paths.first()));

    //painter->drawPath(m_shape);
}

int RawItem::type() const { return RawItemType; }

Paths RawItem::paths() const { return m_paths; }

QPainterPath RawItem::shape() const { return m_shape; }

const G::File* RawItem::file() const { return m_file; }

//RawItem::RawItem(GraphicObject& item)
//    : aperture(item.gFile->apertures[item.state.curAperture])
//    , size(item.gFile->apertures[item.state.curAperture /*lstAperture*/]->size())
//    , imgPolarity(item.state.imgPolarity)
//{
//    //size = item.gFile->apertures.take(item.state.curAperture /*lstAperture*/).Size();
//    setFlags(ItemIsSelectable);
//    setAcceptHoverEvents(true);
//    if (imgPolarity == POSITIVE)
//        color = QColor(255, 180, 120);
//    else
//        color = QColor(128, 128, 0);

//    for (Path& path : item.paths) {
//        m_shape.addPolygon(PathToQPolygon(path));
//    }

//    QString toolTipStr;
//    int j = 0;
//    m_type = item.state.type;
//    switch (m_type) {
//    case APERTURE:
//        if (imgPolarity == POSITIVE)
//            color = QColor(200, 200, 180);
//        toolTipStr = QString("Aperture: %1").arg(item.state.curAperture);
//        for (Path& pth : item.paths) {
//            path.addPolygon(PathToQPolygon(pth));
//        }
//        //path.addPolygon(PathToQPolygon(item.paths[i]));
//        break;
//    case LINE:
//        for (QPointF& pt : PathToQPolygon(item.path)) {
//            j++ ? path.lineTo(pt) : path.moveTo(pt);
//        }
//        toolTipStr = QString("Aperture: %1").arg(item.state.curAperture /*lstAperture*/);
//        break;
//    case REGION:
//        toolTipStr = QString("Aperture: %1").arg(item.state.curAperture /*lstAperture*/);
//        path.addPolygon(PathToQPolygon(item.path));
//        break;
//    }
//    for (QString str : item.gerberStrings) {
//        toolTipStr += '\n' + str;
//    }
//    setToolTip(toolTipStr);
//    rect = m_shape.boundingRect();
//}

//void RawItem::paint(QPainter* painter,
//    const QStyleOptionGraphicsItem* option,
//    QWidget* widget)
//{
//    Q_UNUSED(widget);
//    QColor fillColor;
//    if (option->state & QStyle::State_Selected) {
//        fillColor = QColor(255, 50, 50);
//    }
//    else {
//        fillColor = color;
//    }
//    if (option->state & QStyle::State_MouseOver)
//        if (imgPolarity == POSITIVE)
//            fillColor = fillColor.lighter(120);
//        else
//            fillColor = fillColor.darker(200);

//    const double scale = 1.0 / painter->worldTransform().m11();
//    switch (m_type) {
//    case APERTURE:
//        painter->setBrush(fillColor);
//        painter->setPen(QPen(fillColor, 0.0));
//        painter->drawPath(path);
//        break;
//    case LINE:
//        painter->setBrush(Qt::NoBrush);
//        if (size < scale)
//            painter->setPen(QPen(fillColor, 0.0));
//        else
//            painter->setPen(QPen(fillColor, size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
//        painter->drawPath(path);
//        break;
//    case REGION:
//        painter->setBrush(fillColor);
//        painter->setPen(QPen(fillColor, 0.0));
//        painter->drawPath(path);
//        break;
//    default:
//        painter->setBrush(fillColor);
//        painter->setPen(QPen(fillColor, 0.0));
//        painter->drawPath(path);
//        break;
//    }
//    //    painter->setBrush(fillColor);
//    //    painter->setPen(QPen(fillColor, 0.0));
//    //    painter->drawPath(path);
//}
