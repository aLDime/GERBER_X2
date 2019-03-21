#include "toolpathcreator.h"
#include "forms/materialsetupform.h"
#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QSettings>
#include <algorithm>
#include <gi/bridgeitem.h>
#include <myscene.h>

void fixBegin(Path& path)
{
    IntPoint p = path.first();
    int s = 0;
    for (int i = 1, end = path.size(); i < end; ++i) {
        if (p.Y >= path[i].Y) {
            p.Y = path[i].Y;
            s = i;
            if (p.X > path[i].X) {
                p.X = path[i].X;
                s = i;
            }
        }
    }
    if (s)
        std::rotate(path.begin(), path.begin() + s, path.end());
}

Paths sortByStratDistance(Paths src)
{
    Paths dst;
    dst.reserve(src.size());
    IntPoint p1(MaterialSetup::homePos.x() * uScale, MaterialSetup::homePos.y() * uScale);
    while (src.size()) {
        int s = 0;
        IntPoint p2;
        double l1 = Length(p1, p2);
        for (int i = 0; i < src.size(); ++i) {
            p2 = src[i].first();
            double l2 = Length(p1, p2);
            if (l1 > l2) {
                l1 = l2;
                s = i;
            }
        }
        dst.append(src.takeAt(s));
        p1 = dst.last().first();
    }
    return dst;
}

ToolPathCreator::ToolPathCreator(const Paths& value)
    : m_workingPaths(value)
{
}

GCodeFile* ToolPathCreator::createPocket(const Tool& tool, bool convent, double depth, bool side, int steps)
{

    double toolDiameter = tool.getDiameter(depth) * uScale;
    double dOffset = toolDiameter / 2;
    double stepOver = tool.stepover * uScale;

    if (side) {
        groupedPaths(CutoffPaths, toolDiameter + 5);
    } else
        groupedPaths(CopperPaths);

    ClipperOffset offset(uScale, uScale / 1000);
    Clipper clipper;
    Paths tmpPaths;
    Paths fillPaths;
    Pathss sortedPathss;
    if (1) {
        for (Paths paths : m_groupedPaths) {
            offset.Clear();
            offset.AddPaths(paths, /*jtMiter*/ jtRound, etClosedPolygon);
            offset.Execute(paths, -dOffset);

            fillPaths.append(paths);
            if (steps) {
                int counter = steps;
                if (counter > 1) {
                    do {
                        if (counter == 1)
                            fillPaths.append(paths);
                        tmpPaths.append(paths);
                        offset.Clear();
                        offset.AddPaths(paths, jtMiter, etClosedPolygon);
                        offset.Execute(paths, -stepOver);
                    } while (paths.size() && --counter);
                } else {
                    tmpPaths.append(paths);
                    fillPaths.append(paths);
                }
            } else {
                do {
                    tmpPaths.append(paths);
                    offset.Clear();
                    offset.AddPaths(paths, jtMiter, etClosedPolygon);
                    offset.Execute(paths, -stepOver);
                } while (paths.size());
            }

            clipper.Clear();
            clipper.AddPaths(tmpPaths, ptSubject, true);
            IntRect r(clipper.GetBounds());
            int k = tool.diameter * uScale;
            Path outer = {
                IntPoint(r.left - k, r.bottom + k),
                IntPoint(r.right + k, r.bottom + k),
                IntPoint(r.right + k, r.top - k),
                IntPoint(r.left - k, r.top - k)
            };

            PolyTree polyTree;
            clipper.AddPath(outer, ptSubject, true);
            clipper.Execute(ctUnion, polyTree, pftEvenOdd);
            grouping(polyTree.GetFirst(), &sortedPathss, CopperPaths);
            for (Paths paths : sortedPathss) {
                m_returnPaths.append(paths);
            }
            tmpPaths.clear();
            sortedPathss.clear();
        }
    } else {
    }

    if (m_returnPaths.size() == 0)
        return nullptr;

    if (convent) {
        for (Path& path : m_returnPaths) {
            if (Orientation(path))
                ReversePath(path);
            else
                fixBegin(path);
        }
    } else {
        for (Path& path : m_returnPaths) {
            if (!Orientation(path))
                ReversePath(path);
            else
                fixBegin(path);
        }
    }

    std::reverse(m_returnPaths.begin(), m_returnPaths.end());

    return new GCodeFile(sortByStratDistance(m_returnPaths), tool, depth, Pocket, fillPaths);
}

QVector<GCodeFile*> ToolPathCreator::createPocket2(const QVector<Tool>& /*tool*/, bool /*convent*/, double /*depth*/, bool /*side*/, int /*steps*/)
{
    QVector<GCodeFile*> gcf;
    return gcf;
}

GCodeFile* ToolPathCreator::createProfile(const Tool& tool, bool convent, double depth, SideOfMilling side)
{

    const double toolDiameter = tool.getDiameter(depth);

    if (side == On) {
        // execute offset
        m_returnPaths = m_workingPaths;
        if (!convent)
            ReversePaths(m_returnPaths);
    } else {
        double dOffset;
        // calc offset
        if (side == Outer)
            dOffset = +toolDiameter * uScale * 0.5;
        else
            dOffset = -toolDiameter * uScale * 0.5;

        // execute offset
        ClipperOffset offset;
        for (Paths& paths : groupedPaths(CopperPaths))
            offset.AddPaths(paths, jtRound, etClosedPolygon);

        offset.Execute(m_returnPaths, dOffset);

        if (m_returnPaths.size() == 0)
            return nullptr;

        // fix direction
        if (side == Outer && convent)
            ReversePaths(m_returnPaths);
        else if (side == Inner && !convent)
            ReversePaths(m_returnPaths);
    }

    for (Path& path : m_returnPaths)
        fixBegin(path);

    for (Path& path : m_returnPaths)
        if (path.first() != path.last())
            path.append(path.first());

    ////////////////////////////////////
    QVector<BridgeItem*> brItems;
    for (QGraphicsItem* item : MyScene::self->items()) {
        if (item->type() == BridgeType) {
            brItems.append(static_cast<BridgeItem*>(item));
        }
    }

    if (brItems.size()) {
        Paths paths(m_returnPaths);
        m_returnPaths.clear();
        for (int i = 0; i < paths.size(); ++i) {
            for (BridgeItem* bi : brItems) {
                if (side != On) {
                    qDebug() << "angle" << bi->angle();
                    if (PointOnPolygon(bi->getPoint(1), paths[i])) {
                        qDebug("PointOnPolygon 111");
                    } else if (PointOnPolygon(bi->getPoint(2), paths[i])) {
                        qDebug("PointOnPolygon 222");
                    } else {
                        m_returnPaths.append(paths[i]);
                    }
                } else if (PointOnPolygon(bi->getPoint(0), paths[i])) {
                    qDebug("PointOnPolygon 333");
                } else {
                    m_returnPaths.append(paths[i]);
                }
            }
        }
    }

    return new GCodeFile(sortByStratDistance(m_returnPaths), tool, depth, Profile);
}

Pathss& ToolPathCreator::groupedPaths(Grouping group, cInt k, bool fl)
{
    PolyTree polyTree;
    Clipper clipper;
    clipper.AddPaths(m_workingPaths, ptSubject, true);
    IntRect r(clipper.GetBounds());
    //int k = /*uScale*/ 1;
    Path outer = {
        IntPoint(r.left - k, r.bottom + k),
        IntPoint(r.right + k, r.bottom + k),
        IntPoint(r.right + k, r.top - k),
        IntPoint(r.left - k, r.top - k)
    };
    if (fl)
        ReversePath(outer);
    clipper.AddPath(outer, ptSubject, true);
    clipper.Execute(ctUnion, polyTree, pftNonZero);
    grouping(polyTree.GetFirst(), &m_groupedPaths, group);
    return m_groupedPaths;
}

void ToolPathCreator::grouping(PolyNode* node, Pathss* pathss, Grouping group)
{
    Path path;
    Paths paths;
    switch (group) {
    case CutoffPaths:
        if (!node->IsHole()) {
            path = node->Contour;
            paths.push_back(path);
            for (int i = 0, end = node->ChildCount(); i < end; ++i) {
                path = node->Childs[i]->Contour;
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }

        for (int i = 0, end = node->ChildCount(); i < end; ++i)
            grouping(node->Childs[i], pathss, group);

        break;
    case CopperPaths:
        if (node->IsHole()) {
            path = node->Contour;
            paths.push_back(path);
            for (int i = 0, end = node->ChildCount(); i < end; ++i) {
                path = node->Childs[i]->Contour;
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }

        for (int i = 0, end = node->ChildCount(); i < end; ++i)
            grouping(node->Childs[i], pathss, group);

        break;
    }
}
