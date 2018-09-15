#ifndef TOOLPATHUTIL_H
#define TOOLPATHUTIL_H

#include "tooldatabase/tool.h"
#include <QVector>

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

private:
    const QString m_name;
};

#endif // TOOLPATHUTIL_H
