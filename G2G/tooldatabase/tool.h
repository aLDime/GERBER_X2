#ifndef TOOL_H
#define TOOL_H

#include <QIcon>
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
    Type type = Group;
    double angle = 0.0;
    double diameter = 0.0;
    double feedRate = 0.0;
    double oneTurnCut = 0.0;
    double passDepth = 0.0;
    double plungeRate = 0.0;
    int spindleSpeed = 0.0;
    double stepover = 0.0;

    int id = -1;

    double getDiameter(double depth) const;

    void read(const QJsonObject& json);
    void write(QJsonObject& json) const;
    bool isValid();
    QIcon icon();
    QString errorStr();
    void errorMessageBox(QWidget* parent = nullptr) { QMessageBox::warning(parent, "No valid tool...!!!", errorStr()); }
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
