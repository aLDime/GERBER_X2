#ifndef GERBERPARSER_H
#define GERBERPARSER_H

#include "file.h"
#include "gerber.h"
#include <QObject>
namespace G {
class Parser : public QObject {
    Q_OBJECT
public:
    Parser(QObject* parent = 0);
    void parseFile(const QString& fileName);
    void parseLines(const QString& gerberLines, const QString& fileName);

signals:
    void fileReady(File* file);
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
    void clearStep();

    IntPoint parsePosition(const QString& xyStr);
    Path arc(const IntPoint& center, double radius, double start, double stop);
    Path arc(IntPoint p1, IntPoint p2, IntPoint center);

    Paths createLine();
    Paths createPolygon();

    ClipperLib::Clipper clipper;
    ClipperLib::ClipperOffset offset;

    QMap<QString, QString> apertureMacro;

    bool ab = false;
    Path path;
    State state;
    File* file;
    QList<QString> gerbLines;

    struct
    {
        int x = 0;
        int y = 0;
        double i = 0.0;
        double j = 0.0;
        StepRepeat state = SrClose;
        QList<GraphicObject> acc;
    } sr;

    bool parseAperture(const QString& gLine);
    bool parseApertureBlock(const QString& gLine);

    bool parseStepRepeat(const QString& gLine);
    void closeStepRepeat();

    bool parseApertureMacros(const QString& gLine);
    bool parseAttributes(const QString& gLine);
    bool parseCircularInterpolation(const QString& gLine);
    bool parseEndOfFile(const QString& gLine);
    bool parseFormat(const QString& gLine);
    bool parseGCode(const QString& gLine);
    bool parseImagePolarity(const QString& gLine);
    bool parseLevelPolarity(const QString& gLine);
    bool parseLineInterpolation(const QString& gLine);
    bool parseOperationDCode(const QString& gLine);
    bool parseToolAperture(const QString& gLine);
    bool parseUnitMode(const QString& gLine);
};
}
#endif // GERBERPARSER_H
