#ifndef TOOLPATHUTIL_H
#define TOOLPATHUTIL_H

#include "gcode/toolpathcreator.h"
#include "tooldatabase/tool.h"
#include <QThread>
#include <QVector>
#include <QWidget>

class QProgressDialog;

class ToolPathUtil : public QWidget {
    Q_OBJECT

public:
    ToolPathUtil(const QString& name, QWidget* parent = nullptr);
    ~ToolPathUtil();

protected:
    void readTools(const QVector<Tool*>& tool) const;
    void writeTools(const QVector<Tool*>& tool) const;
    virtual void create() = 0;
    virtual void updateName() = 0;

    void setFile(GCodeFile* file);

    Tool tool;
    Tool tool2;
    Direction direction = Climb;
    SideOfMilling side = Outer;
    Side boardSide = Top;

    ToolPathCreator* toolPathCreator(const Paths& value, const bool convent, SideOfMilling side);
    QString m_fileName;

    void progress(int max, int value);

private:
    const QString m_name;
    QThread thread;
    GCodeFile* m_file;
    void cancel();
    ToolPathCreator* m_tps = nullptr;
    QProgressDialog* pd = nullptr;
    int m_timerId = 0;

    // QObject interface
protected:
    virtual void timerEvent(QTimerEvent* event) override;

};

#endif // TOOLPATHUTIL_H
