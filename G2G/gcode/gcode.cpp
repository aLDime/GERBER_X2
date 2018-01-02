#include "gcode.h"
#include "gcode.h"

#include <QPainter>
#include <QtMath>
#include <QDebug>
#include <QStyleOptionGraphicsItem>
#include <QSettings>
#include <QFile>

GCode::GCode()
{
}

///////////////////////////////////////////////
/// \brief GerberWorkItem::GerberWorkItem
/// \param paths
///
class ggg : public QGraphicsItem {
public:
    ggg(const Paths& paths, const Tool& tool, double m_depth)
        : paths(paths)
        , tool(tool)
        , m_depth(m_depth)
        , d(tool.data.params[Diameter])
    {
        setAcceptHoverEvents(true);
        setFlag(ItemIsSelectable, true);

        if (m_depth > 0.0 && tool.data.params[SideAngle] > 0.0) {
            double a = qDegreesToRadians(90 - tool.data.params[SideAngle] / 2);
            d = (m_depth * cos(a) / sin(a));
            d = d * 2 + tool.data.params[Diameter];
        }

        Paths tmpPaths(paths);
        ClipperOffset offset(uScale, uScale / 1000);

        offset.AddPaths(paths, jtRound, etClosedLine);

        double td = tool.data.params[Diameter];

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

        painter->setPen(QPen(c, tool.data.params[Diameter], Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawPath(m_drawShape);

        if (m_depth > 0.0 && tool.data.params[SideAngle] > 0.0) {
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
    double m_depth;
    double d;
};
///////////////////////////////////////////////
/// \brief GCodeProfile::GCodeProfile
/// \param paths
/// \param tool
/// \param m_depth
///
GCodeProfile::GCodeProfile(const Paths& paths, const Tool& tool, double m_depth)
    : paths(paths)
    , tool(tool)
    , m_depth(m_depth)
    , d(tool.data.params[Diameter])
{
    //   setAcceptHoverEvents(true);
    //    setFlag(ItemIsSelectable, true);

    if (m_depth > 0.0 && tool.data.params[SideAngle] > 0.0) {
        double a = qDegreesToRadians(90 - tool.data.params[SideAngle] / 2);
        d = (m_depth * cos(a) / sin(a));
        d = d * 2 + tool.data.params[Diameter];
    }
    QGraphicsPolygonItem* polygonItem;

    for (const Path& path : paths) {
        //        polygonItem = new QGraphicsPolygonItem(PathToQPolygon(path));
        //        polygonItem->setPen(QPen(QColor(255, 255, 255, 100), tool.data.Params[Diameter], Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        //        polygonItem->setBrush(Qt::NoBrush);
        //        addToGroup(polygonItem);
        //        if (m_depth > 0.0 && tool.data.Params[SideAngle] > 0.0) {
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

void GCodeProfile::save(const QString &name)
{
    QList<QString> sl;
    QVector<QPolygonF> paths(PathsToQPolygons(paths));

    QSettings settings;
    settings.beginGroup("ToolpathNameForm");
    QPointF homePoint(settings.value("HomeXY").toPointF());
    QPointF zeroPoint(settings.value("ZeroXY").toPointF());
    homePoint -= zeroPoint;
    double homeZ(settings.value("HomeZ").toDouble());
    double safeZ(settings.value("SafeZ").toDouble());
    settings.endGroup();
    qDebug() << homePoint << homeZ << safeZ;

    sl.append("G17"); //XY
    sl.append(QString("G0Z%1").arg(homeZ, 0, 'f', 3)); //HomeZ
    sl.append(QString("G0X%1Y%2S%3M3").arg(homePoint.x(), 0, 'f', 3).arg(homePoint.y(), 0, 'f', 3).arg(tool.data.spindleRpm)); //HomeXY
    QPointF lastPoint;
    for (QPolygonF& path : paths) {
        QPointF point(path.last());
        point -= zeroPoint;
        sl.append(QString("G0X%1Y%2").arg(point.x(), 0, 'f', 3).arg(point.y(), 0, 'f', 3)); //start xy
        sl.append(QString("G1Z%1F%2").arg(-m_depth, 0, 'f', 3).arg(tool.data.params[PlungeRate], 0, 'f', 3)); //start z
        bool fl = true;
        for (QPointF& point : path) {
            QString str("G1");
            point -= zeroPoint;
            if (lastPoint.x() != point.x())
                str.append(QString("X%1").arg(point.x(), 0, 'f', 3));
            if (lastPoint.y() != point.y())
                str.append(QString("Y%1").arg(point.y(), 0, 'f', 3));
            if (fl) {
                str.append(QString("F%1").arg(tool.data.params[FeedRate], 0, 'f', 3));
                fl = false;
            }
            sl.append(str);
            lastPoint = point;
        }
        sl.append(QString("G0Z%1").arg(safeZ, 0, 'f', 3));
    }
    sl.append(QString("G0Z%1").arg(homeZ, 0, 'f', 3));
    sl.append(QString("G0X%1Y%2").arg(homePoint.x(), 0, 'f', 3).arg(homePoint.y(), 0, 'f', 3));
    sl.append("M30");

    QFile file(name);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (QString& s : sl) {
            out << s << endl;
        }
    }
}
