#include "abstractnode.h"
#include "project.h"
#include <QDebug>
#include <mainwindow.h>

AbstractNode::AbstractNode()
{
    if (MainWindow::self) {
        MainWindow::self->closeAllAct->setEnabled(true);
        MainWindow::self->exportPdfAct->setEnabled(true);
    }
}

AbstractNode::~AbstractNode()
{
    if (MainWindow::self) {
        MainWindow::self->closeAllAct->setEnabled(Project::isEmpty());
        MainWindow::self->exportPdfAct->setEnabled(Project::isEmpty());
    }
    childItems.clear();
}

int AbstractNode::row() const
{
    if (m_parentItem)
        for (int i = 0, size = m_parentItem->childItems.size(); i < size; ++i)
            if (m_parentItem->childItems[i].data() == this)
                return i;
    return 0;
}

AbstractNode* AbstractNode::child(int row) { return childItems.value(row).data(); }

AbstractNode* AbstractNode::parentItem() { return m_parentItem; }

void AbstractNode::append(AbstractNode* item)
{
    item->m_parentItem = this;
    childItems.append(QSharedPointer<AbstractNode>(item));
}

void AbstractNode::remove(int row) { childItems.removeAt(row); }

int AbstractNode::childCount() const { return childItems.count(); }
