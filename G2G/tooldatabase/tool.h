#ifndef TOOL_H
#define TOOL_H

#include <QIcon>
#include <QJsonObject>
#include <QObject>

class Tool {
public:
    Tool();
    ~Tool();

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
    double spindleSpeed = 0.0;
    double stepover = 0.0;

    int id = -1;

    double getDiameter(double depth) const;

    void read(const QJsonObject& json);
    void write(QJsonObject& json) const;
    bool isValid();
    QIcon icon();
    QString errorStr();
};

Q_DECLARE_METATYPE(Tool)

#endif // TOOL_H
