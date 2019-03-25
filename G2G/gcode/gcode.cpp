#include "gcode.h"

#include "forms/materialsetupform.h"
#include <QFile>
#include <QPainter>
#include <QTextStream>
#include <gi/itemgroup.h>
#include <graphicsview.h>
#include <settingsdialog.h>

///////////////////////////////////////////////
void performance(QVector<QPair<cInt, cInt>>& range, Pathss& pathss, const Paths& paths, bool fl = true)
{
    static cInt top = 0;
    static cInt bottom = 0;
    Clipper clipper;
    clipper.AddPaths(paths, ptSubject, true);
    const IntRect rect(clipper.GetBounds());

    if (fl) {
        top = rect.top - 1;
        bottom = rect.bottom + 1;
    }

    const cInt k = rect.right - rect.left;
    Paths paths1;
    Paths paths2;
    qDebug() << ((rect.bottom - rect.top) * dScale);
    if (k < (uScale * 10)) {
        range.append(qMakePair(rect.left, rect.right));
        pathss.append(paths);
    } else {
        cInt c = static_cast<cInt>(k * 0.5);
        Path outerLeft{
            IntPoint(rect.left - 1, top),
            IntPoint(rect.left + c + 1, top),
            IntPoint(rect.left + c + 1, bottom),
            IntPoint(rect.left - 1, bottom)
        };
        clipper.Clear();
        clipper.AddPaths(paths, ptSubject, false);
        clipper.AddPath(outerLeft, ptClip, true);
        clipper.Execute(ctIntersection, paths1, pftPositive);
        performance(range, pathss, paths1, false);

        Path outerRight{
            IntPoint(rect.right - c - 1, top),
            IntPoint(rect.right + 1, top),
            IntPoint(rect.right + 1, bottom),
            IntPoint(rect.right - c - 1, bottom)
        };
        clipper.Clear();
        clipper.AddPaths(paths, ptSubject, false);
        clipper.AddPath(outerRight, ptClip, true);
        clipper.Execute(ctIntersection, paths2, pftPositive);
        performance(range, pathss, paths2, false);
    }
}

GCodeFile::GCodeFile(const Paths& toolPaths, const Tool& tool, double depth, GCodeType type, const Paths& pocketPaths)
    : m_pocketPaths(pocketPaths)
    , m_type(type)
    , m_toolPaths(toolPaths)
    , m_tool(tool)
    , m_depth(depth)
{

    setItemGroup(new ItemGroup);
    GraphicsItem* item;
    Path g0path;
    Paths g0path1;
    Paths tmpPaths2(toolPaths);

    switch (type) {
    case Profile:
        for (const Path& path : tmpPaths2) {
            item = new PathItem({ path });
            item->setPen(QPen(Qt::black, tool.getDiameter(depth), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            item->setPenColor(SettingsDialog::color(Colors::CutArea));
            itemGroup()->append(item);
        }

        g0path.reserve(toolPaths.size() * 2);
        for (int i = 0; i < tmpPaths2.size(); ++i) {
            item = new PathItem({ tmpPaths2[i] });
            item->setPenColor(SettingsDialog::color(Colors::ToolPath));
            itemGroup()->append(item);
            if (i < tmpPaths2.size() - 1) {
                g0path1.append({ tmpPaths2[i].last(), tmpPaths2[i + 1].first() });
            }
        }

        //        for (const Path& path : tmpPaths2) {
        //            item = new PathItem({ path });
        //
        //            item->setPenColor(SettingsDialog::color(Colors::ToolPath));
        //            itemGroup()->append(item);
        //            g0path.append(path.first());
        //        }

        item = new PathItem(g0path1);
        item->setPenColor(SettingsDialog::color(Colors::G0));
        itemGroup()->append(item);
        break;
    case Pocket:
        if (0) { //fast rendeer
            Paths tmpPaths;
            Pathss pathss;
            Clipper clipper;
            QVector<QPair<cInt, cInt>> range;
            item = new GerberItem(pocketPaths, nullptr);
            item->setPen(QPen(Qt::black, tool.getDiameter(depth), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            item->setPenColor(SettingsDialog::color(Colors::CutArea));
            item->setBrushColor(SettingsDialog::color(Colors::CutArea));
            item->setAcceptHoverEvents(false);
            item->setFlag(QGraphicsItem::ItemIsSelectable, false);
            g0path.reserve(toolPaths.size());
            itemGroup()->append(item);
            clipper.Clear();
            clipper.AddPaths(tmpPaths2, ptSubject, true);
            const IntRect rect(clipper.GetBounds());
            for (Path& path : tmpPaths2)
                if (path.first() != path.last())
                    path.append(path.first());

            cInt ky = uScale * 10;
            performance(range, pathss, tmpPaths2);
            qDebug() << range.size() << pathss.size();
            for (int i = 0; i < range.size(); ++i) {
                for (cInt y = rect.top; y < rect.bottom; y += ky) {
                    qDebug() << i << y;
                    Path outer{
                        IntPoint(range[i].first - 1, y - 1),
                        IntPoint(range[i].second + 1, y - 1),
                        IntPoint(range[i].second + 1, y + ky + 1),
                        IntPoint(range[i].first - 1, y + ky + 1)
                    };
                    clipper.Clear();
                    clipper.AddPaths(pathss[i], ptSubject, false);
                    clipper.AddPath(outer, ptClip, true);
                    clipper.Execute(ctIntersection, tmpPaths, /*pftNonZero*/ pftPositive);
                    item = new PathItem(tmpPaths);

                    item->setPenColor(SettingsDialog::color(Colors::ToolPath));
                    item->setAcceptDrops(false);
                    item->setAcceptedMouseButtons(Qt::NoButton);
                    item->setAcceptHoverEvents(false);
                    item->setAcceptTouchEvents(false);
                    //item->setActive(false);\ g
                    itemGroup()->append(item);
                }
            }

            for (const Path& path : toolPaths)
                g0path.append(path.first());
            item = new PathItem({ g0path });
            item->setPenColor(SettingsDialog::color(Colors::G0));
            itemGroup()->append(item);
        } else {
            item = new GerberItem(pocketPaths, nullptr);
            item->setPen(QPen(SettingsDialog::color(Colors::CutArea), tool.getDiameter(depth), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            item->setBrush(SettingsDialog::color(Colors::CutArea));
            item->setAcceptHoverEvents(false);
            item->setFlag(QGraphicsItem::ItemIsSelectable, false);
            itemGroup()->append(item);
            g0path.reserve(toolPaths.size());
            int i = 0;
            for (Path& path : tmpPaths2) {
                item = new PathItem({ path });
                item->setPenColor(SettingsDialog::color(Colors::ToolPath));
                itemGroup()->append(item);
                if (i < tmpPaths2.size() - 1) {
                    g0path1.append({ tmpPaths2[i].last(), tmpPaths2[++i].first() });
                }
            }
            item = new PathItem({ g0path1 });
            item->setPenColor(SettingsDialog::color(Colors::G0));
            itemGroup()->append(item);
        }
        break;
    case Drilling:
        for (const IntPoint& point : toolPaths.first()) {
            item = new DrillItem(tool.diameter);
            item->setPos(toQPointF(point));
            item->setPenColor(SettingsDialog::color(Colors::ToolPath));
            item->setBrushColor(SettingsDialog::color(Colors::CutArea));
            itemGroup()->append(item);
        }
        item = new PathItem(toolPaths);
        item->setPenColor(SettingsDialog::color(Colors::G0));
        itemGroup()->append(item);
        break;
    default:
        break;
    }
    itemGroup()->addToTheScene();
}

Paths GCodeFile::getPaths() const { return m_toolPaths; }

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
    QPolygonF path(toQPolygon(m_toolPaths.first()));

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
    QVector<QPolygonF> paths(toQPolygons(m_toolPaths));

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
                if (!qFuzzyCompare(lastPoint.x(), point.x()))
                    str += x(point.x());
                if (!qFuzzyCompare(lastPoint.y(), point.y()))
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
            if (!qFuzzyCompare(lastPoint.x(), point.x()))
                str += x(point.x());
            if (!qFuzzyCompare(lastPoint.y(), point.y()))
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

    //    QPointF home(MaterialSetup::homePos - MaterialSetup::zeroPos);
    //    sl.append(g0() + x(home.x()) + y(home.y()) + s(m_tool.spindleSpeed) + "M3"); //HomeXY
    sl.append(s(m_tool.spindleSpeed) + "M3"); //HomeXY
}

void GCodeFile::endFile()
{
    sl.append(g0() + z(MaterialSetup::z)); //HomeZ

    QPointF home(MaterialSetup::homePos - MaterialSetup::zeroPos);
    sl.append(g0() + x(home.x()) + y(home.y()) + s(m_tool.spindleSpeed) + "M3"); //HomeXY

    sl.append("M30");
    QString str(m_fileName);
    QFile file(str.insert(str.length() - 4, QString("(Top)|(Bot)").split('|')[side()]));
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (QString& s : sl)
            out << s << endl;
    }
    file.close();
}
