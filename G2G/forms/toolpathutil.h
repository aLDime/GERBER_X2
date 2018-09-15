#ifndef TOOLPATHUTIL_H
#define TOOLPATHUTIL_H

#include "gcode/toolpathcreator.h"
#include "tooldatabase/tool.h"
#include <QVector>

enum Direction {
    Climb,
    Conventional
};

class ToolPathUtil {
public:
    ToolPathUtil(const QString& name);
    ~ToolPathUtil();
    void readTools(const QVector<Tool*>& tool) const;
    void writeTools(const QVector<Tool*>& tool) const;
    virtual void create() = 0;

protected:
    Tool tool;
    Tool tool2;
    Direction direction = Climb;
    SideOfMilling side = Outer;

private:
    const QString m_name;
};

#endif // TOOLPATHUTIL_H
