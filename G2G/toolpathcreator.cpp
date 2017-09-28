#include "toolpathcreator.h"
#include <QCoreApplication>

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

Paths& ToolPathCreator::Merge(GERBER_FILE* gerberFile)
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
        if (gerberFile->at(i - 1).state.imgPolarity == POSITIVE) {
            clipper.Execute(ctUnion, paths, pftPositive);
        }
        else {
            clipper.Execute(ctDifference, paths, pftNonZero);
        }
    }
    mergedPaths = paths;
    return mergedPaths;
}

Pathss& ToolPathCreator::ToolPathPocket(MILLING milling, double toolDiameter)
{
    double dOffset = -toolDiameter * uScale / 2.00;
    double stepOver = dOffset * 0.8;

    Paths paths;
    Pathss retPaths;

    ClipperOffset offset(uScale, uScale / 1000);

    Clipper clipper;
    clipper.AddPaths(mergedPaths, ptSubject, true);
    IntRect r = clipper.GetBounds();

    int k = uScale;
    Path outer(4);
    outer[0] = IntPoint(r.left - k, r.bottom + k);
    outer[1] = IntPoint(r.right + k, r.bottom + k);
    outer[2] = IntPoint(r.right + k, r.top - k);
    outer[3] = IntPoint(r.left - k, r.top - k);

    //ReversePath(board);
    if (1) {
        GetGroupedPaths((GROUP)milling);
        for (Pathss::size_type i = 0; i < groupedPaths.size(); ++i) {
            paths = groupedPaths[i];
            qDebug() << "groupedPolygons" << i;
            for (int p = 0; p < 1000 && paths.size(); ++p) {
                offset.Clear();
                offset.AddPaths(paths, p ? jtMiter : jtRound, etClosedPolygon);
                offset.Execute(paths, p ? stepOver : dOffset);
                for (Paths::size_type j = 0; j < paths.size(); ++j) {
                    paths[j].push_back(paths[j][0]);
                }
                retPaths.push_back(paths);
                qDebug() << "paths" << paths.size() << p << "groupedPolygons.size()" << groupedPaths.size() << i;
                qApp->processEvents(QEventLoop::AllEvents, 1000);
            }
        }

        PolyTree polyTree;

        clipper.Clear();
        for (Paths::size_type i = 0; i < retPaths.size(); ++i) {
            clipper.AddPaths(retPaths[i], ptSubject, true);
        }
        IntRect r = clipper.GetBounds();

        int k = uScale;
        Path outer(4);
        outer[0] = IntPoint(r.left - k, r.bottom + k);
        outer[1] = IntPoint(r.right + k, r.bottom + k);
        outer[2] = IntPoint(r.right + k, r.top - k);
        outer[3] = IntPoint(r.left - k, r.top - k);

        clipper.Clear();
        for (Paths::size_type i = 0; i < retPaths.size(); ++i) {
            clipper.AddPaths(retPaths[i], ptSubject, true);
        }
        clipper.AddPath(outer, ptSubject, true);
        clipper.Execute(ctXor, polyTree, pftNonZero);
        retPaths.clear();
        grouping(polyTree.GetFirst(), &retPaths, (GROUP)!milling);
    }
    else {
        paths = mergedPaths;
        if (milling) {
            paths.push_back(outer);
        }
        for (int p = 0; p < 1000 && paths.size() > 0; ++p) {
            offset.Clear();
            offset.AddPaths(paths, jtRound, etClosedPolygon);
            offset.Execute(paths, dOffset * uScale);
            for (Paths::size_type j = 0; j < paths.size(); ++j) {
                paths[j].push_back(paths[j][0]);
            }
            retPaths.push_back(paths);
            qApp->processEvents(QEventLoop::AllEvents, 1000);
        }
    }
    return retPaths;
}

Paths& ToolPathCreator::ToolPathProfile(MILLING milling, double toolDiameter)
{
    double dOffset;
    switch (milling) {
    case OUTSIDE_MILLING:
        dOffset = toolDiameter * uScale * 0.5;
        break;
    case INSIDE_MILLING:
        dOffset = -toolDiameter * uScale * 0.5;
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
    for (Path& path : tmpPaths) {
        path.append(path.first());
    }
    return tmpPaths;
}

constexpr Paths& ToolPathCreator::GetMergedPaths()
{
    return mergedPaths;
}

Pathss& ToolPathCreator::GetGroupedPaths(GROUP group, bool fl)
{
    PolyTree polyTree;
    Clipper clipper;
    clipper.AddPaths(mergedPaths, ptSubject, true);
    IntRect r(clipper.GetBounds());
    int k = uScale;
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

void ToolPathCreator::grouping(PolyNode* n, Pathss* p, GROUP group)
{
    Path path;
    Paths paths;
    switch (group) {
    case CUTOFF:
        if (!n->IsHole()) {
            path = n->Contour;
            paths.push_back(path);
            for (int var = 0; var < n->ChildCount(); ++var) {
                path = n->Childs[var]->Contour;
                paths.push_back(path);
            }
            p->push_back(paths);
        }
        for (int var = 0; var < n->ChildCount(); ++var) {
            grouping(n->Childs[var], p, group);
        }
        break;
    case COPPER:
        if (n->IsHole()) {
            path = n->Contour;
            paths.push_back(path);
            for (int var = 0; var < n->ChildCount(); ++var) {
                path = n->Childs[var]->Contour;
                paths.push_back(path);
            }
            p->push_back(paths);
        }
        for (int var = 0; var < n->ChildCount(); ++var) {
            grouping(n->Childs[var], p, group);
        }
        break;
    }
}
