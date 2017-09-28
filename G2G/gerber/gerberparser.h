#ifndef GERBERPARSER_H
#define GERBERPARSER_H

#include "gerber.h"
#include "gerberaperture.h"

#include <QObject>

class GERBER_FILE;
class GERBER_ITEM {
public:
    GERBER_ITEM(
        const STATE& state,
        const Paths& paths,
        GERBER_FILE* gFile,
        const QStringList& gerberStrings = QStringList(),
        const Path& path = Path())
        : state(state)
        , paths(paths)
        , gFile(gFile)
        , gerberStrings(gerberStrings)
        , path(path)
    {
    }

    STATE state;
    Paths paths;
    GERBER_FILE* gFile = nullptr;
    QStringList gerberStrings;
    Path path;
};

class GERBER_FILE : public QList<GERBER_ITEM> {
public:
    QList<QString> lines;
    QMap<int, GerberAperture> apertures;
    QString fileName;
};

class GerberParser : public QObject {
    Q_OBJECT
public:
    GerberParser(QObject* parent = 0);
    GERBER_FILE* ParseFile(const QString& fileName);
    GERBER_FILE* ParseLines(const QString& gerberLines, const QString& fileName = QString());

signals:
    void resultReady(GERBER_FILE* gerbFile);

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

    Path curPath;

    STATE state;
    GERBER_FILE* gerbFile;
    QList<QString> gerbLines;

    bool ParseAperture(const QString& gLine);
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

#endif // GERBERPARSER_H
