#ifndef GCODE_H
#define GCODE_H

#include <QGraphicsItemGroup>
#include <QObject>
#include <abstractfile.h>
#include <gerber.h>
#include <gi/itemgroup.h>
#include <myclipper.h>
#include <tooldatabase/tool.h>

enum GCodeType {
    Profile,
    Pocket,
    Drilling,
    Material
};

class GCodeFile : public AbstractFile {
public:
    GCodeFile(const Paths& toolPaths, const Tool& tool, double depth, GCodeType type, const Paths& pocketPaths = {});

    Paths getPaths() const;
    void save(const QString& name = QString());

    GCodeType gtype() const;

    FileType type() const override { return FileType::GCode; }
    Paths m_pocketPaths;

private:
    void saveDrill();
    void saveProfilePocket();

    const GCodeType m_type;

    const Paths m_toolPaths;
    const Tool m_tool;
    double m_depth;

    enum GCode {
        G_null = -1,
        G00 = 0,
        G01 = 1,
        G02 = 2,
        G03 = 3,
    };

    GCode m_gCode = G_null;
    inline QString g0()
    {
        if (m_gCode != G00) {
            m_gCode = G00;
            return "G0";
        }
        return "";
    }
    inline QString g1()
    {
        if (m_gCode != G01) {
            m_gCode = G01;
            return "G1";
        }
        return "";
    }
    inline QString x(double val) { return "X" + format(val); }
    inline QString y(double val) { return "Y" + format(val); }
    inline QString z(double val) { return "Z" + format(val); }
    inline QString feed(double val) { return "F" + format(val); }
    inline QString s(int val) { return "S" + QString::number(val); }
    inline QString format(double val) { return QString::number(val, 'f', 3); }

    void startPath(const QPointF& point);
    void endPath();
    void statFile();
    void endFile();

    QList<QString> sl;

protected:
    virtual Paths merge() const { return m_toolPaths; }
};

#endif // GCODE_H
