#include "toolpathforminterface.h"
#include <QDebug>

ToolpathFormInterface::ToolpathFormInterface()
    : action(nullptr)
{
}

ToolpathFormInterface::~ToolpathFormInterface()
{
    qDebug() << "~ToolpathFormInterface";
}

QAction* ToolpathFormInterface::getAction() const
{
    return action;
}

void ToolpathFormInterface::setAction(QAction* value)
{
    action = value;
}
