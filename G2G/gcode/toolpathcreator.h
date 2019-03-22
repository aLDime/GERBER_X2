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
    ToolPathCreator(const Paths& value);

    GCodeFile* createPocket(const Tool& tool, bool convent, double depth, bool side, int steps);

    QVector<GCodeFile*> createPocket2(const QVector<Tool>& tool, bool convent, double depth, bool side, int steps);

    GCodeFile* createProfile(const Tool& tool, bool convent, double depth, const SideOfMilling side);

    Pathss& groupedPaths(Grouping group, cInt k = 10, bool fl = true);

private:
    const Paths m_workingPaths;
    Paths m_returnPaths;
    Pathss m_groupedPaths;
    void grouping(PolyNode* node, Pathss* pathss, Grouping group);
};

#endif // TOOLPATHCREATOR_H
