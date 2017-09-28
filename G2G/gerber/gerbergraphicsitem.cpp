#include "gerbergraphicsitem.h"
#include "gerbergraphicsitem.h"
#include "graphicsview/mygraphicsscene.h"
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

///////////////////////////////////////////////
/// \brief GerberItem::GerberItem
/// \param item
///
GerberRawItem::GerberRawItem(GERBER_ITEM& item)
    : aperture(&item.gFile->apertures[item.state.curAperture])
    , size(item.gFile->apertures[item.state.lstAperture].Size())
    , imgPolarity(item.state.imgPolarity)
{
    //size = item.gFile->apertures.take(item.state.lstAperture).Size();
    setFlags(ItemIsSelectable);
    setAcceptHoverEvents(true);
    if (imgPolarity == POSITIVE)
        color = QColor(255, 180, 120);
    else
        color = QColor(128, 128, 0);

    for (Path& path : item.paths) {
        m_shape.addPolygon(PathToQPolygon(path));
    }

    QString toolTipStr;
    int j = 0;
    m_type = item.state.type;
    switch (m_type) {
    case APERTURE:
        if (imgPolarity == POSITIVE)
            color = QColor(200, 200, 180);
        toolTipStr = QString("Aperture: %1").arg(item.state.curAperture);
        for (Path& pth : item.paths) {
            path.addPolygon(PathToQPolygon(pth));
        }
        //path.addPolygon(PathToQPolygon(item.paths[i]));
        break;
    case LINE:
        for (QPointF& pt : PathToQPolygon(item.path)) {
            j++ ? path.lineTo(pt) : path.moveTo(pt);
        }
        toolTipStr = QString("Aperture: %1").arg(item.state.lstAperture);
        break;
    case REGION:
        toolTipStr = QString("Aperture: %1").arg(item.state.lstAperture);
        path.addPolygon(PathToQPolygon(item.path));
        break;
    }
    for (QString str : item.gerberStrings) {
        toolTipStr += '\n' + str;
    }
    setToolTip(toolTipStr);
    rect = m_shape.boundingRect();
}

void GerberRawItem::paint(QPainter* painter,
    const QStyleOptionGraphicsItem* option,
    QWidget* widget)
{
    Q_UNUSED(widget);
    QColor fillColor;
    if (option->state & QStyle::State_Selected) {
        fillColor = QColor(255, 50, 50);
    }
    else {
        fillColor = color;
    }
    if (option->state & QStyle::State_MouseOver)
        if (imgPolarity == POSITIVE)
            fillColor = fillColor.lighter(120);
        else
            fillColor = fillColor.darker(200);

    const double scale = 1.0 / painter->worldTransform().m11();
    switch (m_type) {
    case APERTURE:
        painter->setBrush(fillColor);
        painter->setPen(QPen(fillColor, 0.0));
        painter->drawPath(path);
        break;
    case LINE:
        painter->setBrush(Qt::NoBrush);
        if (size < scale)
            painter->setPen(QPen(fillColor, 0.0));
        else
            painter->setPen(QPen(fillColor, size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawPath(path);
        break;
    case REGION:
        painter->setBrush(fillColor);
        painter->setPen(QPen(fillColor, 0.0));
        painter->drawPath(path);
        break;
    default:
        painter->setBrush(fillColor);
        painter->setPen(QPen(fillColor, 0.0));
        painter->drawPath(path);
        break;
    }
    //    painter->setBrush(fillColor);
    //    painter->setPen(QPen(fillColor, 0.0));
    //    painter->drawPath(path);
}

///////////////////////////////////////////////
/// \brief GerberItemGroup::~GerberItemGroup
///
GerberItemGroup::~GerberItemGroup()
{
    qDebug() << "~GerberItemGroup()";
    for (QGraphicsItem* item : *this) {
        QGraphicsScene* scene = item->scene();
        if (scene != nullptr)
            scene->removeItem(item);
        delete item;
    }
}

void GerberItemGroup::addToTheScene(QGraphicsScene* scene)
{
    for (QGraphicsItem* item : *this) {
        scene->addItem(item);
    }
}

void GerberItemGroup::setBrush(const QBrush& brush)
{
    if (m_brush != brush) {
        m_brush = brush;
        for (GerberItem* item : *this) {
            item->setBrush(m_brush);
        }
    }
}

void GerberItemGroup::setPen(const QPen& pen)
{
    if (m_pen != pen) {
        m_pen = pen;
        for (GerberItem* item : *this) {
            item->setPen(m_pen);
        }
    }
}
///////////////////////////////////////////////
/// \brief GerberWorkItem::GerberWorkItem
/// \param paths
///
GerberWorkItem::GerberWorkItem(const Paths& paths)
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

QRectF GerberWorkItem::boundingRect() const { return rect; }

QPainterPath GerberWorkItem::shape() const { return m_shape; }

void GerberWorkItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);
    QPen pen_(Qt::NoPen);
    QBrush brush_(m_brush);
    QColor c(m_brush.color());
    if (option->state & QStyle::State_MouseOver) {
        c.setAlpha(200);
        brush_.setColor(c);
        c.setAlpha(255);
        pen_ = QPen(c, 0.0);
    }
    if (option->state & QStyle::State_Selected) {
        c.setAlpha(255);
        brush_.setColor(c);
    }
    //    if (option->state & QStyle::State_Sunken) {
    //        qDebug() << option->state;
    //        pen_ = QPen(Qt::black, 0.0);
    //    }
    painter->setBrush(brush_);
    painter->setPen(pen_);
    painter->drawPath(m_shape);
}

int GerberWorkItem::type() const { return GerberWorkItemType; }

Paths GerberWorkItem::getPaths() const { return paths; }
