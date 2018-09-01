#include "tool.h"
#include <qmath.h>

Tool::Tool()
    : name("Name")
{
}

Tool::~Tool()
{
}

double Tool::getDiameter(double depth) const
{
    if (depth > 0.0 && angle > 0.0) {
        double a = qDegreesToRadians(90 - angle / 2);
        double d = depth * cos(a) / sin(a);
        return d * 2 + diameter;
    }
    return diameter;
}

void Tool::read(const QJsonObject& json)
{
    angle = json["angle"].toDouble();
    diameter = json["diameter"].toDouble();
    feedRate = json["feedRate"].toDouble();
    oneTurnCut = json["oneTurnCut"].toDouble();
    passDepth = json["passDepth"].toDouble();
    plungeRate = json["plungeRate"].toDouble();
    spindleSpeed = json["spindleSpeed"].toDouble();
    stepover = json["stepover"].toDouble();

    name = json["name"].toString();
    note = json["note"].toString();
    type = static_cast<Type>(json["type"].toInt());
}

void Tool::write(QJsonObject& json) const
{
    json["angle"] = angle;
    json["diameter"] = diameter;
    json["feedRate"] = feedRate;
    json["oneTurnCut"] = oneTurnCut;
    json["passDepth"] = passDepth;
    json["plungeRate"] = plungeRate;
    json["spindleSpeed"] = spindleSpeed;
    json["stepover"] = stepover;

    json["name"] = name;
    json["note"] = note;
    json["type"] = type;
}
