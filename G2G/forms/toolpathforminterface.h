#ifndef TOOLPATHFORMINTERFACE_H
#define TOOLPATHFORMINTERFACE_H

#include <QAction>
#include <QObject>

class ToolpathFormInterface {
public:
    ToolpathFormInterface();
    virtual ~ToolpathFormInterface();

    QAction* getAction() const;
    void setAction(QAction* value);

private:
    QAction* action;
};

#endif // TOOLPATHFORMINTERFACE_H
