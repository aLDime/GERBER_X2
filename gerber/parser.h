#ifndef GERBERPARSER_H
#define GERBERPARSER_H

#include "file.h"
#include "gerber.h"
#include <QObject>
#include <QStack>
namespace G {
class Parser : public QObject {
    Q_OBJECT
public:
    Parser(QObject* parent = 0);
    void parseFile(const QString& fileName);
    void parseLines(const QString& gerberLines, const QString& fileName);

signals:
    void fileReady(File* m_file);
    void fileProgress(const QString& fileName, int max, int value);
    void fileError(const QString& fileName, const QString& error);

private:
    QList<QString> format(QString data);
    double arcAngle(double start, double stop);
    double toDouble(const QString& Str, bool scale = false, bool inchControl = true);
    bool parseNumber(QString Str, cInt& val, int integer, int decimal);

    void addPath();
    void addFlash();

    void reset(const QString& fileName);
    void resetStep();

    IntPoint parsePosition(const QString& xyStr);
    Path arc(const IntPoint& center, double radius, double start, double stop);
    Path arc(IntPoint p1, IntPoint p2, IntPoint center);

    Paths createLine();
    Paths createPolygon();

    ClipperLib::Clipper m_clipper;
    ClipperLib::ClipperOffset m_offset;

    QMap<QString, QString> m_apertureMacro;

    enum WorkingType {
        Normal,
        StepRepeat,
        ApertureBlock,
    };

    QStack<QPair<WorkingType, int>> m_abSrIdStack;

    Path m_path;
    State m_state;
    File* m_file;
    QList<QString> m_gerbLines;
    int m_lineNum = 0;

    struct {
        void reset()
        {
            x = 0;
            y = 0;
            i = 0.0;
            j = 0.0;
            storage.clear();
        }
        int x = 0;
        int y = 0;
        double i = 0.0;
        double j = 0.0;
        QList<GraphicObject> storage;
    } stepRepeat;

    bool parseAperture(const QString& gLine);
    bool parseApertureBlock(const QString& gLine);
    bool parseApertureMacros(const QString& gLine);
    bool parseAttributes(const QString& gLine);
    bool parseCircularInterpolation(const QString& gLine);
    bool parseDCode(const QString& gLine);
    bool parseEndOfFile(const QString& gLine);
    bool parseFormat(const QString& gLine);
    bool parseGCode(const QString& gLine);
    bool parseImagePolarity(const QString& gLine);
    bool parseLineInterpolation(const QString& gLine);
    bool parseStepRepeat(const QString& gLine);
    bool parseTransformations(const QString& gLine);
    bool parseUnitMode(const QString& gLine);
    void closeStepRepeat();

    ApBlock* apb(int id) { return static_cast<ApBlock*>(m_file->m_apertures[id].data()); }
};
}
#endif // GERBERPARSER_H
