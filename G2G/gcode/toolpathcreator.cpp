#include "toolpathcreator.h"
#include "forms/materialsetupform.h"
#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QSettings>
#include <algorithm>
#include <aperture.h>
#include <gi/bridgeitem.h>
#include <scene.h>

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
        p1 = dst.last().last();
    }
    return dst;
}

bool PointOnPolygon(const QLineF& l2, const Path& path, IntPoint* ret = nullptr)
{
    int cnt = path.size();
    if (cnt < 3)
        return false;
    IntPoint pt1 = path[0];
    QPointF p;
    for (int i = 1; i <= cnt; ++i) {
        IntPoint pt2(i == cnt ? path[0] : path[i]);
        QLineF l1(toQPointF(pt1), toQPointF(pt2));
        if (QLineF::BoundedIntersection == l1.intersect(l2, &p)) {
            if (ret)
                *ret = toIntPoint(p);
            return true;
        }
        pt1 = pt2;
    }
    return false;
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

GCodeFile* ToolPathCreator::createProfile(const Tool& tool, bool convent, double depth, const SideOfMilling side)
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

    // find Bridges
    QVector<BridgeItem*> bridgeItems;
    for (QGraphicsItem* item : Scene::self->items()) {
        if (item->type() == BridgeType)
            bridgeItems.append(static_cast<BridgeItem*>(item));
    }
    // create Bridges
    if (bridgeItems.size()) {
        for (int index = 0, size = m_returnPaths.size(); index < size; ++index) {
            Path& path = m_returnPaths[index];
            QList<QPair<BridgeItem*, IntPoint>> biStack;
            for (BridgeItem* bi : bridgeItems) {
                IntPoint pt;
                if (PointOnPolygon(bi->getPath(), path, &pt))
                    biStack.append({ bi, pt });
            }
            if (!biStack.isEmpty()) {
                Paths tmpPaths;
                // create frame
                {
                    ClipperOffset offset;
                    offset.AddPath(path, jtMiter, etClosedLine);
                    offset.Execute(tmpPaths, +toolDiameter * uScale * 0.1);

                    Clipper clipper;
                    clipper.AddPaths(tmpPaths, ptSubject, true);
                    for (const QPair<BridgeItem*, IntPoint>& bip : biStack) {
                        clipper.AddPath(CirclePath((bip.first->lenght() + toolDiameter) * uScale, bip.second), ptClip, true);
                    }
                    clipper.Execute(ctIntersection, tmpPaths, pftPositive);
                }
                // cut toolPath
                {
                    Clipper clipper;
                    clipper.AddPath(path, ptSubject, false);
                    clipper.AddPaths(tmpPaths, ptClip, true);
                    PolyTree polytree;
                    clipper.Execute(ctDifference, polytree, pftNonZero);
                    PolyTreeToPaths(polytree, tmpPaths);
                }
                // merge result toolPaths
                for (int i = 0; i < tmpPaths.size(); ++i) {
                    for (int j = 0; j < tmpPaths.size(); ++j) {
                        if (tmpPaths[i].last() == tmpPaths[j].first()) {
                            tmpPaths[i].append(tmpPaths[j]);
                            tmpPaths.remove(j);
                            i = 0;
                            break;
                        }
                        if (tmpPaths[i].first() == tmpPaths[j].last()) {
                            tmpPaths[j].append(tmpPaths[i]);
                            tmpPaths.remove(i);
                            i = 0;
                            break;
                        }
                    }
                }
                --size;
                m_returnPaths.remove(index--);
                m_returnPaths.append(tmpPaths);
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
