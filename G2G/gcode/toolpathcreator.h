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

    friend class Clipper;

public:
    static ToolPathCreator* self;
    ToolPathCreator(const Paths& value, const bool convent, SideOfMilling side);
    ~ToolPathCreator();
    void createPocket(const Tool& tool, const double depth, const int steps);
    void createPocket2(const QPair<Tool, Tool>& tool, double depth);
    void createProfile(const Tool& tool, double depth);
    void createTermal(Gerber::File* file, const Tool& tool, double depth);
    void createVoronoi(const Tool& tool, double depth, const double k, bool test);
    Pathss& groupedPaths(Grouping group, cInt k = 10, bool fl = true);
    void addRawPaths(Paths rawPaths);
    void addPaths(const Paths& paths);

    GCodeFile* file() const;

signals:
    void fileReady(GCodeFile* file);
    void progress(int max, int value);

private:
    GCodeFile* m_file = nullptr;
    Paths m_workingPaths;
    Paths m_workingRawPaths;
    Paths m_returnPaths;
    Pathss m_groupedPaths;
    void grouping(PolyNode* node, Pathss* pathss, Grouping group);

    Path& fixPath(PolyNode* node);
    void grouping2(PolyNode* node, Paths* addRawPaths, bool fl = false);

    inline bool progressOrCancel(int max, int value, int skipKey);
    static void progressOrCancel();
    int max = 1000000;
    int value = 0;
    //    void DoOffset(const Paths& addRawPaths, Pathss& pathss)
    //    {
    //        if (paths.isEmpty())
    //            return;
    //        static bool fl = false;
    //        Paths wPaths;

    //        QVector<bool> flags;
    //        for (const Path& path : paths)
    //            flags.append(Orientation(path));
    //        qDebug() << flags;

    //        if (flags.contains(false)) {
    //            qDebug() << "NNN";
    //            ClipperOffset offset;
    //            offset.AddPaths(paths, fl ? jtMiter : jtRound, etClosedPolygon);
    //            offset.Execute(wPaths, -m_stepOver);
    //            fl = true;
    //            if (wPaths.isEmpty())
    //                return;
    //            Paths tmp(wPaths);
    //            //        for (Path& path : tmp) {
    //            //            //path.append(path.first());
    //            //            //            if (!m_convent)
    //            //            //                ReversePath(path);
    //            //        }
    //            if (paths.size() == tmp.size()) {
    //                bool fl = true;
    //                for (int i = 0; i < tmp.size() && fl; ++i) {
    //                    if (flags[i])
    //                        fl = PointInPolygon(tmp[i][0], pathss.last()[i]) > 0;
    //                    else
    //                        fl = PointInPolygon(pathss.last()[i][0], tmp[i]) > 0;
    //                }
    //                if (fl) {
    //                    qDebug() << "append N";
    //                    for (int i = 0; i < tmp.size(); ++i)
    //                        pathss.last()[i].append(tmp[i]);
    //                    DoOffset(wPaths, pathss);
    //                } else {
    //                    qDebug() << "insert N1";
    //                    pathss.append(tmp);
    //                    DoOffset(wPaths, pathss);
    //                }
    //            } else {
    //                qDebug() << "insert N2";
    //                pathss.append(tmp);
    //                DoOffset(wPaths, pathss);
    //            }
    //        } else {
    //            for (const Path& path : paths) {
    //                qDebug() << "111";
    //                ClipperOffset offset;
    //                offset.AddPath(path, fl ? jtMiter : jtRound, etClosedPolygon);
    //                offset.Execute(wPaths, -m_stepOver);
    //                fl = true;
    //                if (wPaths.isEmpty())
    //                    return;
    //                Paths tmp(wPaths);
    //                for (Path& path : tmp) {
    //                    flags.append(Orientation(path));
    //                    //path.append(path.first());
    //                    //                if (!m_convent)
    //                    //                    ReversePath(path);
    //                }
    //                if (wPaths.size() == 1) {
    //                    qDebug() << "append";
    //                    if (PointInPolygon(tmp[0][0], pathss.last()[0]))
    //                        pathss.last()[0].append(tmp[0]);
    //                    else
    //                        pathss.append({ tmp[0] });
    //                    DoOffset(wPaths, pathss);
    //                } else {
    //                    for (int i = 0; i < tmp.size(); ++i) {
    //                        qDebug() << "insert";
    //                        pathss.append({ tmp[i] });
    //                        DoOffset({ wPaths[i] }, pathss);
    //                    }
    //                }
    //            }
    //        }
    //    }
    const SideOfMilling m_side;
    double m_toolDiameter = 0.0;
    double m_dOffset = 0.0;
    double m_stepOver = 0.0;
    const bool m_convent;
};

#endif // TOOLPATHCREATOR_H
