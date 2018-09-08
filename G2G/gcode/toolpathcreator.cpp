#include "toolpathcreator.h"
#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QSettings>
#include <algorithm>
#include <forms/materialsetupform.h>
//GERBER_FILE ToolPathCreator::file;

//Paths ToolPathCreator::mergedPolygons;
//vPaths ToolPathCreator::groupedPolygons;

void fixBegin(Path& path)
{
    QElapsedTimer t;
    t.start();
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
    qDebug() << t.nsecsElapsed() << s << path.size();
}

Paths sortByStratDistance(Paths src)
{
    Paths dst;
    dst.reserve(src.size());
    IntPoint p1(MaterialSetupForm::homePos.x() * uScale, MaterialSetupForm::homePos.y() * uScale);
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
{
    mergedPaths = value;
}





GCode* ToolPathCreator::ToolPathPocket(/*MILLING milling,*/ const QVector<Tool>& tool, bool convent, double depth)
{
    double toolDiameter = tool[0].diameter;
    double dOffset = toolDiameter * (uScale / 2);
    double stepOver = tool[0].stepover * uScale;

    mergedPaths;

    GetGroupedPaths(CUTOFF, true);

    ClipperOffset offset(uScale, uScale / 1000);
    Clipper clipper;
    Paths paths_1;
    Pathss paths_2;

    tmpPaths.clear();
    for (Paths paths : groupedPaths) {
        offset.Clear();
        offset.AddPaths(paths, jtMiter /*jtRound*/, etClosedPolygon);
        offset.Execute(paths, -dOffset);
        do {
            paths_1.append(paths);
            offset.Clear();
            offset.AddPaths(paths, jtMiter, etClosedPolygon);
            offset.Execute(paths, -stepOver);
        } while (paths.size());

        PolyTree polyTree;
        clipper.Clear();
        clipper.AddPaths(paths_1, ptSubject, true);
        IntRect r(clipper.GetBounds());
        int k = 1;
        Path outer = {
            IntPoint(r.left - k, r.bottom + k),
            IntPoint(r.right + k, r.bottom + k),
            IntPoint(r.right + k, r.top - k),
            IntPoint(r.left - k, r.top - k)
        };
        clipper.AddPath(outer, ptSubject, true);
        clipper.Execute(ctUnion, polyTree, pftEvenOdd);
        grouping(polyTree.GetFirst(), &paths_2, COPPER);
        for (Paths paths : paths_2) {
            tmpPaths.append(paths);
        }
        paths_1.clear();
        paths_2.clear();
    }
    groupedPaths.clear();

    if (tmpPaths.size() == 0)
        return nullptr;

    if (!convent) {
        for (Path& path : tmpPaths) {
            if (Orientation(path))
                ReversePath(path);
            else
                fixBegin(path);
        }
    } else {
        for (Path& path : tmpPaths) {
            if (!Orientation(path))
                ReversePath(path);
            else
                fixBegin(path);
        }
    }

    return new GCode(sortByStratDistance(tmpPaths) /*tmp2*/ /*tmpPaths*/, tool[0], depth, POCKET);
}

GCode* ToolPathCreator::ToolPathProfile(MILLING milling, const Tool& tool, bool convent, double depth)
{
    double toolDiameter = tool.getDiameter(depth);

    double dOffset;
    switch (milling) {
    case OUTSIDE_MILLING:
        dOffset = +toolDiameter * (uScale / 2);
        break;
    case INSIDE_MILLING:
        dOffset = -toolDiameter * (uScale / 2);
        break;
    case ON_MILLING:
        dOffset = 0;
        break;
    default:
        break;
    }

    ClipperOffset offset(uScale, uScale / 1000);

    for (Paths& paths : GetGroupedPaths(COPPER, true)) {
        offset.AddPaths(paths, jtRound, etClosedPolygon);
    }

    offset.Execute(tmpPaths, dOffset);
    if (tmpPaths.size() == 0)
        return nullptr;

    switch (milling) {
    case OUTSIDE_MILLING:
        if (convent)
            ReversePaths(tmpPaths);
        break;
    case INSIDE_MILLING:
        if (!convent)
            ReversePaths(tmpPaths);
        break;
    case ON_MILLING:
        if (convent)
            ReversePaths(tmpPaths);
        break;
    default:
        break;
    }

    for (Path& path : tmpPaths)
        fixBegin(path);

    return new GCode(sortByStratDistance(tmpPaths) /*tmpPaths*/ /*tmp2*/, tool, depth, PROFILE);
}

Paths ToolPathCreator::GetMergedPaths()
{
    return mergedPaths;
}

Pathss& ToolPathCreator::GetGroupedPaths(GROUP group, bool fl)
{
    PolyTree polyTree;
    Clipper clipper;
    clipper.AddPaths(mergedPaths, ptSubject, true);
    IntRect r(clipper.GetBounds());
    int k = /*uScale*/ 1;
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
    grouping(polyTree.GetFirst(), &groupedPaths, group);
    return groupedPaths;
}

void ToolPathCreator::grouping(PolyNode* node, Pathss* pathss, GROUP group)
{
    Path path;
    Paths paths;
    switch (group) {
    case CUTOFF:
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
    case COPPER:
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
