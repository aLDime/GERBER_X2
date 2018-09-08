#ifndef TOOLPATHCREATOR_H
#define TOOLPATHCREATOR_H

#include <QObject>
#include <file.h>
#include <gcode/gcode.h>
#include <myclipper.h>
#include <tooldatabase/tool.h>

using namespace ClipperLib;

enum MILLING {
    OUTSIDE_MILLING,
    INSIDE_MILLING,
    ON_MILLING,
};

enum GROUP {
    COPPER,
    CUTOFF,
};

class ToolPathCreator {
public:
    ToolPathCreator(const Paths& value);
    //    ToolPathCreator(G::File* gerberFile)
    //    {
    //        Paths paths;
    //        Paths tmpPaths;
    //        int i = 0, exp = -1;
    //        Clipper clipper(ioStrictlySimple);

    //        while (i < gerberFile->size()) {
    //            clipper.Clear();
    //            clipper.AddPaths(paths, ptSubject, true);
    //            exp = gerberFile->at(i).state.imgPolarity;
    //            do {
    //                tmpPaths = gerberFile->at(i++).paths;
    //                //            SimplifyPolygons(tmpPaths, pftNonZero);
    //                clipper.AddPaths(tmpPaths, ptClip, true);
    //            } while (i < gerberFile->size() && exp == gerberFile->at(i).state.imgPolarity);

    //            if (gerberFile->at(i - 1).state.imgPolarity == G::POSITIVE)
    //                clipper.Execute(ctUnion, paths, pftPositive);
    //            else
    //                clipper.Execute(ctDifference, paths, pftNonZero);
    //        }
    //        mergedPaths = paths;
    //    }

    GCode* ToolPathPocket(const QVector<Tool>& tool, bool convent, double depth);
    GCode* ToolPathProfile(MILLING milling, const Tool& tool, bool convent, double depth);
    Paths GetMergedPaths();

    Pathss& GetGroupedPaths(GROUP group, bool fl = false);

private:
    Paths mergedPaths;
    Paths tmpPaths;
    Pathss groupedPaths;
    void grouping(PolyNode* node, Pathss* pathss, GROUP group);
};

#endif // TOOLPATHCREATOR_H
