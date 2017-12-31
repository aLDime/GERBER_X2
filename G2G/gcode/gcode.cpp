#include "gcode.h"
#include "gcode.h"

#include <QPainter>
#include <QtMath>
#include <QDebug>
#include <QStyleOptionGraphicsItem>

GCode::GCode()
{
}

///////////////////////////////////////////////
/// \brief GerberWorkItem::GerberWorkItem
/// \param paths
///
class ggg : public QGraphicsItem {
public:
    ggg(const Paths& paths, const Tool& tool, double cutDepth)
        : paths(paths)
        , tool(tool)
        , cutDepth(cutDepth)
        , d(tool.data.Params[Diameter])
    {
        setAcceptHoverEvents(true);
        setFlag(ItemIsSelectable, true);

        if (cutDepth > 0.0 && tool.data.Params[SideAngle] > 0.0) {
            double a = qDegreesToRadians(90 - tool.data.Params[SideAngle] / 2);
            d = (cutDepth * cos(a) / sin(a));
            d = d * 2 + tool.data.Params[Diameter];
        }

        Paths tmpPaths(paths);
        ClipperOffset offset(uScale, uScale / 1000);

        offset.AddPaths(paths, jtRound, etClosedLine);

        double td = tool.data.Params[Diameter];

        qDebug() << d << td;
        if (d > td)
            td = d;
        offset.Execute(tmpPaths, td * uScale / 2);

        for (Path& path : tmpPaths) {
            path.append(path.first());
            m_shape.addPolygon(PathToQPolygon(path));
        }

        rect = m_shape.boundingRect();
        qDebug() << rect;

        for (Path& path : this->paths) {
            path.append(path.first());
            m_drawShape.addPolygon(PathToQPolygon(path));
        }
    }

    QRectF boundingRect() const override { return rect; }
    QPainterPath shape() const override { return m_shape; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override
    {
        Q_UNUSED(widget);

        //    painter->setCompositionMode(QPainter::CompositionMode_Xor);

        painter->setBrush(Qt::NoBrush);

        QColor c(255, 255, 255, 100);

        if (option->state & QStyle::State_MouseOver) {
            c.setAlpha(150);
        }
        if (option->state & QStyle::State_Selected) {
            c.setAlpha(200);
        }

        painter->setPen(QPen(c, tool.data.Params[Diameter], Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawPath(m_drawShape);

        if (cutDepth > 0.0 && tool.data.Params[SideAngle] > 0.0) {
            painter->setPen(QPen(c, d, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->drawPath(m_drawShape);
        }

        painter->setPen(QPen(Qt::white, 0.0));
        painter->drawPath(m_drawShape);
    }
    int type() const { return QGraphicsItem::UserType + 3; }
    Paths getPaths() const { return paths; }

private:
    QPainterPath m_shape;
    QPainterPath m_drawShape;
    Paths paths;
    QRectF rect;
    Tool tool;
    double cutDepth;
    double d;
};
///////////////////////////////////////////////
/// \brief GCodeProfile::GCodeProfile
/// \param paths
/// \param tool
/// \param cutDepth
///
GCodeProfile::GCodeProfile(const Paths& paths, const Tool& tool, double cutDepth)
    : paths(paths)
    , tool(tool)
    , cutDepth(cutDepth)
    , d(tool.data.Params[Diameter])
{
    //   setAcceptHoverEvents(true);
    //    setFlag(ItemIsSelectable, true);

    if (cutDepth > 0.0 && tool.data.Params[SideAngle] > 0.0) {
        double a = qDegreesToRadians(90 - tool.data.Params[SideAngle] / 2);
        d = (cutDepth * cos(a) / sin(a));
        d = d * 2 + tool.data.Params[Diameter];
    }
    QGraphicsPolygonItem* polygonItem;

    for (const Path& path : paths) {
        //        polygonItem = new QGraphicsPolygonItem(PathToQPolygon(path));
        //        polygonItem->setPen(QPen(QColor(255, 255, 255, 100), tool.data.Params[Diameter], Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        //        polygonItem->setBrush(Qt::NoBrush);
        //        addToGroup(polygonItem);
        //        if (cutDepth > 0.0 && tool.data.Params[SideAngle] > 0.0) {
        polygonItem = new QGraphicsPolygonItem(PathToQPolygon(path));
        polygonItem->setPen(QPen(QColor(255, 255, 255, 150), d, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        polygonItem->setBrush(Qt::NoBrush);
        addToGroup(polygonItem);
        //        }
        polygonItem = new QGraphicsPolygonItem(PathToQPolygon(path));
        polygonItem->setPen(QPen(Qt::white, 0.0));
        polygonItem->setBrush(Qt::NoBrush);
        addToGroup(polygonItem);
    }

    //    QGraphicsItemGroup* group = scene->createItemGroup(items);
    //    group->setAcceptHoverEvents(false);
    //    group->setAcceptTouchEvents(false);
    //    group->setAcceptedMouseButtons(Qt::NoButton);
    //    Paths tmpPaths(paths);
    //    ClipperOffset offset(uScale, uScale / 1000);
    //    offset.AddPaths(paths, jtRound, etClosedLine);
    //    double td = tool.data.Params[Diameter];
    //    qDebug() << d << td;
    //    if (d > td)
    //        td = d;
    //    offset.Execute(tmpPaths, td * uScale / 2);
    //    for (Path& path : tmpPaths) {
    //        path.append(path.first());
    //        m_shape.addPolygon(PathToQPolygon(path));
    //    }
    //    rect = m_shape.boundingRect();
    //    qDebug() << rect;
    //    for (Path& path : this->paths) {
    //        path.append(path.first());
    //        m_drawShape.addPolygon(PathToQPolygon(path));
    //    }
}

Paths GCodeProfile::getPaths() const { return paths; }
