#include "graphicsitem.h"
#include "point.h"
#include <QApplication>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <file.h>
#include <mainwindow.h>
#include <myscene.h>

///////////////////////////////////////////////
/// \brief GerberItem::GerberItem
/// \param item
///
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

///////////////////////////////////////////////
/// \brief GerberItemGroup::~GerberItemGroup
///

ItemGroup::~ItemGroup()
{
    qDeleteAll(*this);
}

void ItemGroup::setVisible(const bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;
        for (QGraphicsItem* item : *this)
            item->setVisible(m_visible);
    }
}

void ItemGroup::addToTheScene(QGraphicsScene* scene)
{
    for (QGraphicsItem* item : *this)
        scene->addItem(item);
    MainWindow::self->zero()->resetPos();
    MainWindow::self->home()->resetPos();
}

void ItemGroup::setBrush(const QBrush& brush)
{
    if (m_brush != brush) {
        m_brush = brush;
        for (GraphicsItem* item : *this)
            item->setBrush(m_brush);
    }
}

void ItemGroup::setPen(const QPen& pen)
{
    if (m_pen != pen) {
        m_pen = pen;
        for (GraphicsItem* item : *this)
            item->setPen(m_pen);
    }
}

void ItemGroup::setZValue(qreal z)
{
    for (GraphicsItem* item : *this)
        item->setZValue(z);
}

///////////////////////////////////////////////
/// \brief GerberWorkItem::GerberWorkItem
/// \param paths
///
WorkItem::WorkItem(const Paths& paths, G::File* file)
    : paths(paths)
{
    for (Path& path : this->paths) {
        if (path.first() != path.last())
            path.append(path.first());
        m_shape.addPolygon(PathToQPolygon(path));
    }
    rect = m_shape.boundingRect();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
}

QRectF WorkItem::boundingRect() const { return rect; }

QPainterPath WorkItem::shape() const { return m_shape; }

void WorkItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    QBrush brush(m_brush);
    QColor c(m_brush.color());
    QPen pen(Qt::NoPen);

    if (option->state & QStyle::State_Selected) {
        c.setAlpha(255);
        brush.setColor(c);
        c.setAlpha(100);
        pen = QPen(c, 0.0);
    }
    if (option->state & QStyle::State_MouseOver) {
        //        c.setAlpha(200);
        //        brush.setColor(c);
        c.setAlpha(255);
        pen = QPen(c, 0.0);
    }
    painter->setBrush(brush);
    painter->setPen(pen);
    painter->drawPath(m_shape);
}

int WorkItem::type() const { return WorkItemType; }

Paths WorkItem::getPaths() const { return paths; }
