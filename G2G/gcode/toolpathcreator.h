#ifndef TOOLPATHCREATOR_H
#define TOOLPATHCREATOR_H

#include <QObject>
#include <file.h>
#include <gcode/gcode.h>
#include <myclipper.h>
#include <tooldatabase/tool.h>

using namespace ClipperLib;

enum SideOfMilling {
    Outer,
    Inner,
    On,
};

enum Grouping {
    CopperPaths,
    CutoffPaths,
};

class ToolPathCreator {
public:
    ToolPathCreator(const Paths& value);

    GCode* createPocket(const QVector<Tool>& tool, bool convent, double depth, bool side);
    GCode *createProfile(const Tool& tool, bool convent, double depth, SideOfMilling side);

    Pathss& groupedPaths(Grouping group, cInt k = 10, bool fl = true);

private:
    const Paths m_workingPaths;
    Paths m_returnPaths;
    Pathss m_groupedPaths;
    void grouping(PolyNode* node, Pathss* pathss, Grouping group);
};

#endif // TOOLPATHCREATOR_H
