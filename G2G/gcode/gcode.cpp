#include "gcode.h"

#include "forms/materialsetupform.h"
#include <QFile>
#include <QPainter>
#include <QTextStream>
#include <gi/drillitem.h>
#include <gi/pathitem.h>
#include <mygraphicsview.h>

///////////////////////////////////////////////
/// \brief GCodeProfile::GCodeProfile
/// \param paths
/// \param tool
/// \param m_depth
///
GCode::GCode(const Paths& paths, const Tool& tool, double depth, GCodeType type)
    : m_paths(paths)
    , m_tool(tool)
    , m_depth(depth)
    , m_type(type)
{
    setItemGroup(new ItemGroup);
    PathItem* item;
    DrillItem* itemd;
    Path p;
    p.reserve(paths.size());
    switch (type) {
    case Profile:
    case Pocket:
        for (const Path& path : paths) {
            p.append(path);
            //            item = new PathItem(path);
            //            //item->setPen(QPen(QColor::fromHsvF((0.83333 / paths.size()) * i++, 1.0, 1.0, 0.5), tool.getDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            //            item->setPen(QPen(QColor(100, 100, 100), tool.getDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            //            append(item);
        }
        item = new PathItem(p);
        //item->setPen(QPen(QColor::fromHsvF((0.83333 / paths.size()) * i++, 1.0, 1.0, 0.5), tool.getDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPen(QPen(QColor(100, 100, 100), tool.getDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        itemGroup()->append(item);
        p.clear();
        p.squeeze();
        for (const Path& path : paths) {
            item = new PathItem(path);
            item->setPen(QPen(Qt::black, 0.0, Qt::DashLine));
            //  item->w = d;
            itemGroup()->append(item);
            p.append(path.first());
        }
        item = new PathItem(p);
        item->setPen(QPen(Qt::green, 0.0));
        //  item->w = 10;
        itemGroup()->append(item);
        //  setPen(QPen(QColor(50, 50, 50), tool.getDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        itemGroup()->setBrush(Qt::NoBrush);
        break;
    case Drilling:
        for (const IntPoint& point : paths.first()) {
            itemd = new DrillItem(tool.diameter);
            itemd->setPos(ToQPointF(point));
            itemd->setPen(QPen(Qt::red, 0.0));
            itemd->setBrush(Qt::red);
            itemGroup()->append(itemd);
        }
        item = new PathItem(paths.first());
        item->setPen(QPen(Qt::green, 0.0));
        itemGroup()->append(item);
        break;
    default:
        break;
    }
    itemGroup()->addToTheScene();
}

Paths GCode::getPaths() const { return m_paths; }

void GCode::save(const QString& name)
{
    if (!name.isEmpty())
        m_fileName = name;
    switch (m_type) {
    case Profile:
    case Pocket:
        saveProfilePocket();
        break;
    case Drilling:
        saveDrill();
        break;
    default:
        break;
    }
}

void GCode::saveDrill()
{
    statFile();
    QPolygonF path(PathToQPolygon(m_paths.first()));

    double maxX = -std::numeric_limits<double>::max();
    double minX = +std::numeric_limits<double>::max();

    for (QPointF& point : path) {
        point -= MaterialSetup::zeroPos;
        if (m_side && maxX < point.x())
            maxX = point.x();
        if (m_side && minX > point.x())
            minX = point.x();
    }
    if (m_side) {
        const double k = minX + maxX;
        for (QPointF& point : path) {
            point.rx() = -point.x() + k;
        }
    }

    for (QPointF& point : path) {
        startPath(point);
        for (int i = 1; m_depth > m_tool.passDepth * i; ++i) {
            sl.append(g1() + z(-m_tool.passDepth * i) + feed(m_tool.plungeRate));
            sl.append(QString("G0Z0"));
        }
        sl.append(g1() + z(-m_depth) + feed(m_tool.plungeRate));
        endPath();
    }
    endFile();
}

void GCode::saveProfilePocket()
{
    statFile();
    QVector<QPolygonF> paths(PathsToQPolygons(m_paths));

    double maxX = -std::numeric_limits<double>::max();
    double minX = +std::numeric_limits<double>::max();

    for (QPolygonF& path : paths) {
        for (QPointF& point : path) {
            point -= MaterialSetup::zeroPos;
            if (m_side && maxX < point.x())
                maxX = point.x();
            if (m_side && minX > point.x())
                minX = point.x();
        }
    }
    if (m_side) {
        const double k = minX + maxX;
        for (QPolygonF& path : paths) {
            for (QPointF& point : path) {
                point.rx() = -point.x() + k;
            }
        }
    }

    QPointF lastPoint;

    for (int i = 1; m_depth > m_tool.passDepth * i; ++i) {
        for (QPolygonF& path : paths) {
            QPointF point(path.last());

            startPath(point);

            sl.append(g1() + z(-m_tool.passDepth * i) + feed(m_tool.plungeRate)); //start z

            bool fl = true;
            for (QPointF& point : path) {
                QString str("G1");
                if (lastPoint.x() != point.x())
                    str += x(point.x());
                if (lastPoint.y() != point.y())
                    str += y(point.y());
                if (fl) {
                    str += feed(m_tool.feedRate);
                    fl = false;
                }
                sl.append(str);
                lastPoint = point;
            }
            endPath();
        }
    }

    for (QPolygonF& path : paths) {
        QPointF point(path.last());

        startPath(point);

        sl.append(g1() + z(-m_depth) + feed(m_tool.plungeRate)); //start z

        bool fl = true;
        for (QPointF& point : path) {
            QString str("G1");
            if (lastPoint.x() != point.x())
                str += x(point.x());
            if (lastPoint.y() != point.y())
                str += y(point.y());
            if (fl) {
                str += feed(m_tool.feedRate);
                fl = false;
            }
            sl.append(str);
            lastPoint = point;
        }
        endPath();
    }

    endFile();
}

GCodeType GCode::gtype() const
{
    return m_type;
}

G::Side GCode::side() const
{
    return m_side;
}

void GCode::setSide(const G::Side& side)
{
    m_side = side;
    m_fileName += side ? " (Bottom)" : " (Top)";
}

void GCode::startPath(const QPointF& point)
{
    sl.append(g0() + x(point.x()) + y(point.y())); //start xy
    sl.append(g0() + z(MaterialSetup::plunge)); //start z
}

void GCode::endPath()
{
    sl.append(QString("G0Z%1").arg(format(MaterialSetup::clearence)));
}

void GCode::statFile()
{
    sl.clear();
    sl.append("G17"); //XY plane
    sl.append(g0() + z(MaterialSetup::z)); //HomeZ

    QPointF home(MaterialSetup::homePos - MaterialSetup::zeroPos);
    sl.append(g0() + x(home.x()) + y(home.y()) + s(m_tool.spindleSpeed) + "M3"); //HomeXY
}

void GCode::endFile()
{
    sl.append(g0() + z(MaterialSetup::z)); //HomeZ

    QPointF home(MaterialSetup::homePos - MaterialSetup::zeroPos);
    sl.append(g0() + x(home.x()) + y(home.y()) + s(m_tool.spindleSpeed) + "M3"); //HomeXY

    sl.append("M30");

    QFile file(m_fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (QString& s : sl)
            out << s << endl;
    }
    file.close();
}

////////////////////////////////////////////////////
/// \brief PathItem::PathItem
/// \param path
///

////////////////////////////////////////////////////
/// \brief PathItem::PathItem
/// \param path
///
