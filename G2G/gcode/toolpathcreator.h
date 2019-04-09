#ifndef TOOLPATHCREATOR_H
#define TOOLPATHCREATOR_H

#include <QObject>
#include <file.h>
#include <gcode/gcode.h>
#include <myclipper.h>
#include <tooldatabase/tool.h>

using namespace ClipperLib;

enum SideOfMilling {
    On,
    Outer,
    Inner,
};

enum Grouping {
    CopperPaths,
    CutoffPaths,
};

class ToolPathCreator {
public:
    ToolPathCreator(const Paths& value, const bool convent);
    GCodeFile* createPocket(const Tool& tool, const double depth, const bool side, const int steps, const bool ex);
    QPair<GCodeFile*, GCodeFile*> createPocket2(const QPair<Tool, Tool>& tool, double depth, bool side, int steps);
    GCodeFile* createProfile(const Tool& tool, double depth, const SideOfMilling side);
    Pathss& groupedPaths(Grouping group, cInt k = 10, bool fl = true);
    void addRawPaths(Paths rawPaths);
    void addPaths(const Paths& paths);

private:
    Paths m_workingPaths;
    Paths m_workingRawPaths;
    Paths m_returnPaths;
    Pathss m_groupedPaths;
    void grouping(PolyNode* node, Pathss* pathss, Grouping group);

    Path& fixPath(PolyNode* node);
    void grouping2(PolyNode* node, Paths* addRawPaths, bool fl = false);

    void DoOffset(const Paths& addRawPaths, Pathss& pathss);

    double m_toolDiameter = 0.0;
    double m_dOffset = 0.0;
    double m_stepOver = 0.0;
    const bool m_convent;
};

#endif // TOOLPATHCREATOR_H
