#ifndef TOOLPATHCREATOR_H
#define TOOLPATHCREATOR_H

#include <QObject>
#include <QSemaphore>
#include <gbrfile.h>
#include <gcode/gcode.h>
#include <myclipper.h>
#include <tooldatabase/tool.h>

using namespace ClipperLib;

enum SideOfMilling {
    On,
    Outer,
    Inner,
};

enum Direction {
    Climb,
    Conventional
};

enum Grouping {
    CopperPaths,
    CutoffPaths,
};

void fixBegin(Path& path);

class ToolPathCreator : public QObject {
    Q_OBJECT
    friend class ClipperLib::Clipper;

public:
    static ToolPathCreator* self;
    ToolPathCreator(const Paths& workingPaths, const bool convent, SideOfMilling side);
    ~ToolPathCreator();
    void createPocket(const Tool& tool, const double depth, const int steps);
    void createPocket2(const QPair<Tool, Tool>& tool, double depth);
    void createProfile(const Tool& tool, double depth);
    void createThermal(Gerber::File* file, const Tool& tool, double depth);
    void createVoronoi(const Tool& tool, double depth, const double k);
    Pathss& groupedPaths(Grouping group, cInt k = 10, bool fl = true);
    void addRawPaths(Paths rawPaths);
    void addSupportPaths(Pathss supportPaths);
    void addPaths(const Paths& paths);

    GCodeFile* file() const;

    int progressMax() const;
    int progressValue() const;

signals:
    void fileReady(GCodeFile* file);

private:
    GCodeFile* m_file = nullptr;
    Paths m_workingPaths;
    Paths m_workingRawPaths;
    Paths m_returnPaths;
    Pathss m_supportPathss;
    Pathss m_groupedPathss;
    void grouping(PolyNode* node, Pathss* pathss, Grouping group);

    Path& fixPath(PolyNode* node);
    void grouping2(PolyNode* node, Paths* addRawPaths, bool fl = false);

    inline void progressOrCancel(int progressMax, int progressValue);
    static void progressOrCancel();

    int m_progressMax = 1000000;
    int m_progressValue = 0;

    const SideOfMilling m_side;
    double m_toolDiameter = 0.0;
    double m_dOffset = 0.0;
    double m_stepOver = 0.0;
    const bool m_convent;
};

#endif // TOOLPATHCREATOR_H
