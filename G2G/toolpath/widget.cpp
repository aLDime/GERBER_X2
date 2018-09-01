#include "widget.h"
#include "toolpathwidget.h"
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QSettings>

Widget::Widget(QWidget* parent)
    : QWidget(parent)
{
    ToolPathWidget* w = static_cast<ToolPathWidget*>(parent->parent());
    depth = w->dsbxDepth;
    pathName = w->leNameToolPath;
    toolName[0] = w->lblToolName1;
    toolName[1] = w->lblToolName2;
    panelName = w->lblPanelName;
    cbxTool2 = w->cbxTool2;
}

void Widget::restoreTools(const QVector<Tool*>& tool, const QString& name) const
{
    QFile loadFile(name + ".json");

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open tools file.");
        return;
    }

    QJsonDocument loadDoc(QJsonDocument::fromJson(loadFile.readAll()));
    QJsonObject json = loadDoc.object();
    QJsonArray toolArray = json["tools"].toArray();
    for (int treeIndex = 0; treeIndex < toolArray.size(); ++treeIndex) {
        QJsonObject toolObject = toolArray[treeIndex].toObject();
        tool[treeIndex]->read(toolObject);
        toolName[treeIndex]->setText(tool[treeIndex]->name);
    }
}

void Widget::saveTools(const QVector<Tool*>& tool, const QString& name)
{
    QFile saveFile(name + ".json");
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open tools file.");
        return;
    }
    QJsonObject json;
    QJsonArray toolArray;
    for (int treeIndex = 0; treeIndex < tool.size(); ++treeIndex) {
        QJsonObject toolObject;
        tool[treeIndex]->write(toolObject);
        toolArray.append(toolObject);
    }
    json["tools"] = toolArray;
    QJsonDocument saveDoc(json);
    saveFile.write(saveDoc.toJson());
}

void Widget::setName(const QString& name)
{
    panelName->setText(QString("<html><head/><body><p><span style=\"font-weight:600;\">%1</span></p></body></html>").arg(name));
}
