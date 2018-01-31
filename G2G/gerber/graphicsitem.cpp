#include "graphicsitem.h"
//#include "graphicsitem.h"
//#include "graphicsitem.h"
#include "graphicsview/mygraphicsscene.h"
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDebug>
#include <QStyleOptionGraphicsItem>

using namespace G;

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
    for (QGraphicsItem* item : *this) {
        QGraphicsScene* scene = item->scene();
        if (scene != nullptr)
            scene->removeItem(item);
        delete item;
    }
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
///////////////////////////////////////////////
/// \brief GerberWorkItem::GerberWorkItem
/// \param paths
///
WorkItem::WorkItem(const Paths& paths)
    : paths(paths)
{
    for (Path& path : this->paths) {
        path.append(path.first());
        m_shape.addPolygon(PathToQPolygon(path));
    }
    rect = m_shape.boundingRect();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
}

QRectF WorkItem::boundingRect() const { return rect; }

QPainterPath WorkItem::shape() const { return m_shape; }

void WorkItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);

    //    painter->setCompositionMode(QPainter::CompositionMode_Xor);

    QBrush brush(m_brush);
    QColor c(m_brush.color());
    QPen pen(c, 0.0);

    if (option->state & QStyle::State_MouseOver) {
        c.setAlpha(200);
        brush.setColor(c);
        c.setAlpha(255);
        pen.setColor(c);
    }
    if (option->state & QStyle::State_Selected) {
        c.setAlpha(255);
        brush.setColor(c);
        pen.setColor(c);
    }
    ///////////////////////////////
    //    pen_ = QPen(c, 0.0);
    //    if (option->state & QStyle::State_Sunken) {
    //        qDebug() << option->state;
    //        pen_ = QPen(Qt::black, 0.0);
    //    }
    painter->setBrush(brush);
    painter->setPen(pen);
    painter->drawPath(m_shape);
    //    for (QPolygonF p : m_shape.toSubpathPolygons()) {
    //        painter->drawPolygon(p);
    //    }
}

int WorkItem::type() const { return WorkItemType; }

Paths WorkItem::getPaths() const { return paths; }
