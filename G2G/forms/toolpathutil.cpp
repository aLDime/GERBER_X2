#include "toolpathutil.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

ToolPathUtil::ToolPathUtil(const QString& name)
    : m_name(name + ".json")
{
    readTools({ &tool, &tool2 });
}

ToolPathUtil::~ToolPathUtil()
{
    writeTools({ &tool, &tool2 });
}

void ToolPathUtil::readTools(const QVector<Tool*>& tool) const
{
    QFile file(m_name);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open tools file.");
        return;
    }

    QJsonDocument loadDoc(QJsonDocument::fromJson(file.readAll()));
    QJsonObject json = loadDoc.object();
    QJsonArray toolArray = json["tools"].toArray();
    for (int treeIndex = 0; treeIndex < toolArray.size(); ++treeIndex) {
        QJsonObject toolObject = toolArray[treeIndex].toObject();
        tool[treeIndex]->read(toolObject);
    }
}

void ToolPathUtil::writeTools(const QVector<Tool*>& tool) const
{
    QFile file(m_name);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open tools file.");
        return;
    }

    QJsonArray toolArray;
    for (int treeIndex = 0; treeIndex < tool.size(); ++treeIndex) {
        QJsonObject toolObject;
        tool[treeIndex]->write(toolObject);
        toolArray.append(toolObject);
    }
    QJsonObject json;
    json["tools"] = toolArray;
    QJsonDocument saveDoc(json);
    file.write(saveDoc.toJson());
}
