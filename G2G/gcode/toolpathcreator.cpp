#include "toolpathcreator.h"
#include "forms/materialsetupform.h"
#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QSettings>
#include <algorithm>

void fixBegin(Path& path)
{
    cInt x = path.first().X, y = path.first().Y;
    int s = 1;
    int i = 1;
    for (int end = path.size(); i < end; ++i) {
        if (y >= path[i].Y) {
            y = path[i].Y;
            s = i;
            if (x > path[i].X) {
                x = path[i].X;
                s = i;
            }
        }
    }
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

    for (Paths paths : m_groupedPaths) {
        offset.Clear();
        offset.AddPaths(paths, /*jtMiter*/ jtRound, etClosedPolygon);
        offset.Execute(paths, -dOffset);

        CleanPolygons(paths, 0.0009 * uScale);
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

    if (m_returnPaths.size() == 0)
        return nullptr;

    if (!convent) {
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

    return new GCodeFile(sortByStratDistance(m_returnPaths), tool, depth, Pocket, fillPaths);
}

QVector<GCodeFile*> ToolPathCreator::createPocket2(const QVector<Tool>& tool, bool convent, double depth, bool side, int steps)
{
    QVector<GCodeFile*> gcf;
    return gcf;
}

GCodeFile* ToolPathCreator::createProfile(const Tool& tool, bool convent, double depth, SideOfMilling side)
{

    double toolDiameter = tool.getDiameter(depth);

    double dOffset;
    switch (side) {
    case Outer:
        dOffset = +toolDiameter * (uScale / 2);
        break;
    case Inner:
        dOffset = -toolDiameter * (uScale / 2);
        break;
    case On:
        dOffset = 0;
        break;
    default:
        break;
    }

    ClipperOffset offset;

    for (Paths& paths : groupedPaths(CopperPaths)) {
        offset.AddPaths(paths, jtRound, etClosedPolygon);
    }

    offset.Execute(m_returnPaths, dOffset);
    if (m_returnPaths.size() == 0)
        return nullptr;

    switch (side) {
    case Outer:
        if (convent)
            ReversePaths(m_returnPaths);
        break;
    case Inner:
        if (!convent)
            ReversePaths(m_returnPaths);
        break;
    case On:
        if (convent)
            ReversePaths(m_returnPaths);
        break;
    default:
        break;
    }

    for (Path& path : m_returnPaths)
        fixBegin(path);

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
