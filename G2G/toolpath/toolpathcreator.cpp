#include "toolpathcreator.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QSettings>
#include <QElapsedTimer>

//GERBER_FILE ToolPathCreator::file;

//Paths ToolPathCreator::mergedPolygons;
//vPaths ToolPathCreator::groupedPolygons;

ToolPathCreator::ToolPathCreator(QObject* parent)
    : QObject(parent)
{
}

void ToolPathCreator::Clear()
{
    mergedPaths.clear();
    groupedPaths.clear();
}

Paths ToolPathCreator::Merge(G::File* gerberFile)
{
    Paths paths;
    Paths tmpPaths;
    //    file = gerberFile;
    int i = 0, exp = -1;
    Clipper clipper(ioStrictlySimple);
    while (i < gerberFile->size()) {
        clipper.Clear();
        clipper.AddPaths(paths, ptSubject, true);
        exp = gerberFile->at(i).state.imgPolarity;
        do {
            tmpPaths = gerberFile->at(i++).paths;
            SimplifyPolygons(tmpPaths, pftNonZero);
            clipper.AddPaths(tmpPaths, ptClip, true);
        } while (i < gerberFile->size() && exp == gerberFile->at(i).state.imgPolarity);
        if (gerberFile->at(i - 1).state.imgPolarity == G::POSITIVE) {
            clipper.Execute(ctUnion, paths, pftPositive);
        }
        else {
            clipper.Execute(ctDifference, paths, pftNonZero);
        }
    }
    mergedPaths = paths;
    return mergedPaths;
}

GCodeProfile* ToolPathCreator::ToolPathPocket(/*MILLING milling,*/ const QVector<Tool>& tool, bool convent, double depth)
{
    double toolDiameter = tool[0].data.params[Diameter];
    double dOffset = toolDiameter * (uScale / 2);
    double stepOver = tool[0].data.params[Stepover] * uScale;

    mergedPaths;

    GetGroupedPaths(CUTOFF, true);

    ClipperOffset offset(uScale, uScale / 1000);
    Clipper clipper;
    Paths paths_1;
    Pathss paths_2;

    if (1) {
        tmpPaths.clear();
        for (Paths paths : groupedPaths) {
            offset.Clear();
            offset.AddPaths(paths, jtRound, etClosedPolygon);
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
    }

    if (tmpPaths.size() == 0)
        return nullptr;

    std::reverse(tmpPaths.begin(), tmpPaths.end());

    if (convent)
        for (Path& path : tmpPaths) {
            if (Orientation(path))
                ReversePath(path);
        }
    else
        for (Path& path : tmpPaths) {
            if (!Orientation(path))
                ReversePath(path);
        }

    return new GCodeProfile(tmpPaths, tool[0], depth);
}

GCodeProfile* ToolPathCreator::ToolPathProfile(MILLING milling, const Tool& tool, bool convent, double depth)
{
    double toolDiameter = tool.diameter(depth);

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
    if (!convent)
        ReversePaths(tmpPaths);

    //    for (Path& path : tmpPaths) {
    //        path.append(path.first());
    //    }
    return new GCodeProfile(tmpPaths, tool, depth);
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

ToolPathCreator& ToolPathCreator::addPaths(const Paths& value)
{
    mergedPaths.append(value);
    return *this;
}

ToolPathCreator& ToolPathCreator::setPaths(const Paths& value)
{
    mergedPaths = value;
    return *this;
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
            for (int var = 0; var < node->ChildCount(); ++var) {
                path = node->Childs[var]->Contour;
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }
        for (int var = 0; var < node->ChildCount(); ++var) {
            grouping(node->Childs[var], pathss, group);
        }
        break;
    case COPPER:
        if (node->IsHole()) {
            path = node->Contour;
            paths.push_back(path);
            for (int var = 0; var < node->ChildCount(); ++var) {
                path = node->Childs[var]->Contour;
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }
        for (int var = 0; var < node->ChildCount(); ++var) {
            grouping(node->Childs[var], pathss, group);
        }
        break;
    }
}
