#include "tool.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <qmath.h>

int toolId = qRegisterMetaType<Tool>("Tool");

Tool::Tool()
    : name("Name")
{
}

double Tool::getDiameter(double depth) const
{
    if (type == Engraving && depth > 0.0 && angle > 0.0 && angle < 90.0) {
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
    spindleSpeed = json["spindleSpeed"].toInt();
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

bool Tool::isValid()
{
    bool fl = true;
    if (qFuzzyIsNull(diameter))
        fl = false;
    if (qFuzzyIsNull(passDepth))
        fl = false;
    if (type != Drill) {
        if (qFuzzyIsNull(feedRate))
            fl = false;
        if (qFuzzyIsNull(stepover))
            fl = false;
    }
    if (qFuzzyIsNull(plungeRate))
        fl = false;
    return fl;
}

QIcon Tool::icon()
{
    switch (type) {
    case Tool::Drill:
        return Icon(ToolDrillIcon);
    case Tool::EndMill:
        return Icon(ToolEndmillIcon);
    case Tool::Engraving:
        return Icon(ToolEngraverIcon);
    default:
        return QIcon();
    }
}

QString Tool::errorStr()
{
    QString errorString;
    if (qFuzzyIsNull(diameter))
        errorString += "Tool diameter = 0!\n";
    if (qFuzzyIsNull(passDepth)) {
        if (type == Drill)
            errorString += "Pass = 0!\n";
        else
            errorString += "Depth = 0!\n";
    }
    if (qFuzzyIsNull(feedRate))
        errorString += "Feed rate = 0\n";
    if (qFuzzyIsNull(stepover))
        errorString += "Stepover = 0\n";
    if (qFuzzyIsNull(plungeRate))
        errorString += "Plunge rate = 0!\n";
    return errorString;
}

///////////////////////////////////////////////////////
/// \brief ToolHolder::tools
///
QMap<int, Tool> ToolHolder::tools;

ToolHolder::ToolHolder()
{
}

void ToolHolder::readTools()
{
    QFile loadFile(QStringLiteral("../tools.dat"));
    if (loadFile.exists()) {
        if (!loadFile.open(QIODevice::ReadOnly)) {
            return;
        }
    } else {
        loadFile.setFileName(QStringLiteral("tools.dat"));
        if (!loadFile.open(QIODevice::ReadOnly)) {
            return;
        }
    }

    QJsonDocument loadDoc(QJsonDocument::fromBinaryData(loadFile.readAll()));
    QJsonArray toolArray = loadDoc.object()["tools"].toArray();
    for (int treeIndex = 0; treeIndex < toolArray.size(); ++treeIndex) {
        Tool tool;
        QJsonObject toolObject = toolArray[treeIndex].toObject();
        tool.read(toolObject);
        tool.id = toolObject["id"].toInt();
        tools[tool.id] = tool;
    }
}

void ToolHolder::readTools(const QJsonObject& json)
{
    QJsonArray toolArray = json["tools"].toArray();
    for (int treeIndex = 0; treeIndex < toolArray.size(); ++treeIndex) {
        Tool tool;
        QJsonObject toolObject = toolArray[treeIndex].toObject();
        tool.read(toolObject);
        tool.id = toolObject["id"].toInt();
        tools[tool.id] = tool;
    }
}

void ToolHolder::writeTools(QJsonObject& json)
{
    QJsonArray toolArray;
    QMap<int, Tool>::iterator i = tools.begin();
    while (i != tools.constEnd()) {
        QJsonObject toolObject;
        i.value().write(toolObject);
        toolObject["id"] = i.key();
        toolArray.append(toolObject);
        ++i;
    }
    json["tools"] = toolArray;
}
