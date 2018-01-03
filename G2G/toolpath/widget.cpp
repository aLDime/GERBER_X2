#include "toolpathwidget.h"
#include "widget.h"
#include <QDebug>
#include <QSettings>
#include <QLabel>

Widget::Widget(QWidget* parent)
    : QWidget(parent)
{
    ToolPathWidget* w = static_cast<ToolPathWidget*>(parent->parent());
    depth = w->dsbxDepth;
    pathName = w->leNameToolPath;
    toolName[0] = w->lblToolName1;
    toolName[1] = w->lblToolName2;
    panelName = w->lblPanelName;
}

void Widget::restoreTool(const QVector<Tool*>& tool, const QString& name) const
{
    QSettings settings;
    settings.beginGroup("tools");
    for (int i = 0; i < tool.size(); ++i) {
        QByteArray data(settings.value(name + QString().setNum(i)).toByteArray());
        if (data.size()) {
            tool[i]->fromHex(data.split('|')[0]);
            tool[i]->name = data.split('|')[1];
            tool[i]->note = data.split('|')[2];
            toolName[i]->setText(tool[i]->name);
        }
    }
    settings.endGroup();
}

void Widget::saveTool(const QVector<Tool*>& tool, const QString& name)
{
    QSettings settings;
    settings.beginGroup("tools");
    for (int i = 0; i < tool.size(); ++i) {
        settings.setValue(name + QString().setNum(i), tool[i]->toHex().append('|').append(tool[i]->name).append('|').append(tool[i]->note));
    }
    settings.endGroup();
}

void Widget::setName(const QString &name)
{
    panelName->setText(QString("<html><head/><body><p><span style=\"font-weight:600;\">%1</span></p></body></html>").arg(name));
}
