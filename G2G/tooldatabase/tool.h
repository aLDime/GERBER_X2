#ifndef TOOL_H
#define TOOL_H

#include "icons.h"
#include <QJsonObject>
#include <QMap>
#include <QMessageBox>
#include <QObject>

class Tool {
public:
    Tool();

    enum Type {
        Drill,
        EndMill,
        Engraving,
        Group
    };

    enum {
        Angle,
        Diameter,
        FeedRate,
        OneTurnCut,
        PassDepth,
        PlungeRate,
        SpindleSpeed,
        Stepover,
        OneTurnCutPercent,
        StepoverPercent,
    };

    QString name;
    QString note;
    Type type = EndMill;
    double angle = 0.0;
    double diameter = 1.0;
    double feedRate = 1200.0;
    double oneTurnCut = 0.1;
    double passDepth = 2.0;
    double plungeRate = 1200.0;
    int spindleSpeed = 12000.0;
    double stepover = 0.5;
    bool autoName = true;

    int id = 0;

    double getDiameter(double depth) const;

    void read(const QJsonObject& json);
    void write(QJsonObject& json) const;
    bool isValid();
    QIcon icon();
    QString errorStr();
    void errorMessageBox(QWidget* parent = nullptr) { QMessageBox::warning(parent, QObject::tr("No valid tool...!!!"), errorStr()); }
};

class ToolHolder {
public:
    ToolHolder();

    static void readTools();
    static void readTools(const QJsonObject& json);
    static void writeTools(QJsonObject& json);
    static QMap<int, Tool> tools;
};

Q_DECLARE_METATYPE(Tool)

#endif // TOOL_H
