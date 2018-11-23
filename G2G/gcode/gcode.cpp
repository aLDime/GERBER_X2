#include "gcode.h"

#include "forms/materialsetupform.h"
#include <QFile>
#include <QPainter>
#include <QTextStream>
#include <gi/itemgroup.h>
#include <mygraphicsview.h>

///////////////////////////////////////////////
/// \brief GCodeProfile::GCodeProfile
/// \param paths
/// \param tool
/// \param m_depth
///
GCodeFile::GCodeFile(const Paths& paths, const Paths& paths2, const Tool& tool, double depth, GCodeType type)
    : m_paths(paths)
    , m_paths2(paths2)
    , m_tool(tool)
    , m_depth(depth)
    , m_type(type)
{

    setItemGroup(new ItemGroup);
    GraphicsItem* item;
    Path p;

    switch (type) {
    case Profile:
        for (const Path& path : paths) {
            item = new PathItem(path);
            item->setPen(QPen(QColor(50, 50, 50), tool.getDiameter(depth), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            itemGroup()->append(item);
        }
        p.reserve(paths.size());
        for (const Path& path : paths) {
            item = new PathItem(path);
            item->setPen(QPen(Qt::red, 0.0));
            itemGroup()->append(item);
            p.append(path.first());
        }
        item = new PathItem(p);
        item->setPen(QPen(Qt::green, 0.0));
        itemGroup()->append(item);
        itemGroup()->setBrush(Qt::NoBrush);
        break;
    case Pocket:
        item = new GerberItem(paths2, nullptr);
        item->setPen(QPen(QColor(50, 50, 50), tool.getDiameter(depth), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setBrush(QColor(50, 50, 50));
        item->setAcceptHoverEvents(false);
        item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        p.reserve(paths.size());
        itemGroup()->append(item);
        for (const Path& path : paths) {
            item = new PathItem(path);
            item->setPen(QPen(Qt::red, 0.0));
            item->setAcceptDrops(false);
            item->setAcceptedMouseButtons(false);
            item->setAcceptHoverEvents(false);
            item->setAcceptTouchEvents(false);
            //item->setActive(false);
            itemGroup()->append(item);
            p.append(path.first());
        }
        item = new PathItem(p);
        item->setPen(QPen(Qt::green, 0.0));
        item->setBrush(Qt::NoBrush);
        itemGroup()->append(item);
        break;
    case Drilling:
        for (const IntPoint& point : paths.first()) {
            item = new DrillItem(tool.diameter);
            item->setPos(ToQPointF(point));
            item->setPen(QPen(Qt::red, 0.0));
            item->setBrush(Qt::red);
            itemGroup()->append(item);
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

Paths GCodeFile::getPaths() const { return m_paths; }

void GCodeFile::save(const QString& name)
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

void GCodeFile::saveDrill()
{
    statFile();
    QPolygonF path(PathToQPolygon(m_paths.first()));

    double maxX = -std::numeric_limits<double>::max();
    double minX = +std::numeric_limits<double>::max();

    for (QPointF& point : path)
        point -= MaterialSetup::zeroPos;

    if (m_side) {
        for (QPointF& point : path) {
            if (maxX < point.x())
                maxX = point.x();
            if (minX > point.x())
                minX = point.x();
        }
        const double k = minX + maxX;
        for (QPointF& point : path) {
            point.rx() = -point.x() + k;
        }
    }

    for (QPointF& point : path) {
        qDebug() << "saveDrill" << point << path.size();
        startPath(point);
        for (int i = 1; m_depth > m_tool.passDepth * i; ++i) {
            sl.append(g1() + z(-m_tool.passDepth * i) + feed(m_tool.plungeRate));
            sl.append(QString(g0() + "Z0"));
        }
        sl.append(g1() + z(-m_depth) + feed(m_tool.plungeRate));
        endPath();
    }
    endFile();
}

void GCodeFile::saveProfilePocket()
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
                QString str(g1());
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
            QString str(g1());
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

GCodeType GCodeFile::gtype() const
{
    return m_type;
}

G::Side GCodeFile::side() const
{
    return m_side;
}

void GCodeFile::setSide(const G::Side& side)
{
    m_side = side;
    m_fileName += side ? " (Bottom)" : " (Top)";
}

void GCodeFile::startPath(const QPointF& point)
{
    sl.append(g0() + x(point.x()) + y(point.y())); //start xy
    sl.append(g0() + z(MaterialSetup::plunge)); //start z
}

void GCodeFile::endPath()
{
    sl.append(QString(g0() + "Z%1").arg(format(MaterialSetup::clearence)));
}

void GCodeFile::statFile()
{
    sl.clear();
    sl.append("G21 G17 G90"); //G17 XY plane
    sl.append(g0() + z(MaterialSetup::z)); //HomeZ

    QPointF home(MaterialSetup::homePos - MaterialSetup::zeroPos);
    sl.append(g0() + x(home.x()) + y(home.y()) + s(m_tool.spindleSpeed) + "M3"); //HomeXY
}

void GCodeFile::endFile()
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
