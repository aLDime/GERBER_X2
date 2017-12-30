#ifndef GERBERPARSER_H
#define GERBERPARSER_H

#include "file.h"
#include "gerber.h"
#include <QMutex>
#include <QObject>
namespace G {
class Parser : public QObject {
    Q_OBJECT
public:
    Parser(QObject* parent = 0);
    void parseFile(const QString& fileName);
    void ParseLines(const QString& gerberLines, const QString& fileName = QString());

signals:
    void fileReady(File* file);

private:
    QList<QString> Format(QString data);
    double ArcAngle(double start, double stop);
    double Double(const QString& Str, bool scale = false, bool inchControl = true);
    bool ParseNumber(QString Str, cInt& val, int integer, int decimal);

    void ClosePath();
    void CreateFlash();
    void Reset(const QString& fileName);
    void ClearStep();

    IntPoint ParsePosition(const QString& xyStr);
    Path Arc(const IntPoint& center, double radius, double start, double stop);
    Path Arc2(IntPoint p1, IntPoint p2, IntPoint center);

    Paths CreateLine();
    Paths CreatePolygon();

    ClipperLib::Clipper clipper;
    ClipperLib::ClipperOffset offset;

    QMap<QString, QString> apertureMacro;
    //    QMap<int, GerberAperture> apertures;

    bool ab = false;
    Path path;
    State state;
    File* file;
    QList<QString> gerbLines;

    bool ParseAperture(const QString& gLine);
    bool ParseApertureBlock(const QString& gLine);

    bool ParseApertureMacros(const QString& gLine);
    bool ParseAttributes(const QString& gLine);
    bool ParseCircularInterpolation(const QString& gLine);
    bool ParseEndOfFile(const QString& gLine);
    bool ParseFormat(const QString& gLine);
    bool ParseGCode(const QString& gLine);
    bool ParseImagePolarity(const QString& gLine);
    bool ParseLevelPolarity(const QString& gLine);
    bool ParseLineInterpolation(const QString& gLine);
    bool ParseOperationDCode(const QString& gLine);
    bool ParseStepAndRepeat(const QString& gLine);
    bool ParseToolAperture(const QString& gLine);
    bool ParseUnitMode(const QString& gLine);
};
}
#endif // GERBERPARSER_H
