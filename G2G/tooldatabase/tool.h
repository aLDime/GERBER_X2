#ifndef TOOL_H
#define TOOL_H

#include <QObject>

enum ToolType {
    Group = -1,
    EndMill,
    Engraving,
    Drill
};

enum ToolInfo {
    ClearencePassStepover,
    ClearencePassStepoverPercent,
    Diameter,
    FeedRate,
    //    FlatDiameter,
    PassDepth = FeedRate + 2,
    PlungeRate,
    SideAngle,
    Stepover,
    StepoverPercent
};

Q_DECLARE_METATYPE(ToolType)

struct Tool {
public:
    struct D {
        double params[10] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
        //    double ClearencePassStepover;
        //    double ClearencePassStepoverPercent;
        //    double Diameter;
        //    double FeedRate;
        //    double FlatDiameter;
        //    double PassDepth;
        //    double PlungeRate;
        //    double SideAngle;
        //    double Stepover;
        //    double StepoverPercent;
        int feedSpeeds = 1;
        int spindleRpm = 0;
        ToolType toolType = Group;
    } data;

    QString name;
    QString note;

    Tool();
    Tool(const QString& name, const QString& note, const QByteArray& Data);
    Tool(const QList<QString>& Data);
    ~Tool();
    void fromHex(const QByteArray& Data);
    QByteArray toHex() const;

    double diameter(double depth = 0.0) const;
};

Q_DECLARE_METATYPE(Tool)

#endif // TOOL_H
