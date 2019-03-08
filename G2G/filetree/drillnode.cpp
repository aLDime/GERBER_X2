#include "drillnode.h"
#include "gerbernode.h"
#include "staticholders/fileholder.h"

#include <QFileInfo>
#include <mainwindow.h>

DrillNode::DrillNode(DrillFile* file)
    : m_id(FileHolder::addFile(file))
{
    FileHolder::file(m_id)->itemGroup()->addToTheScene();
    //FileHolder::file<Drill>(m_id)->itemGroup()->setZValue(-m_id);
    MyGraphicsView::self->zoomFit();
    MyGraphicsView::self->zoom100();
    MainWindow::self->closeAllAct->setEnabled(true);
}

DrillNode::~DrillNode()
{
    FileHolder::deleteFile(m_id);
    MainWindow::self->closeAllAct->setEnabled(FileHolder::isEmpty());
    if (MyScene::self) {
        MyScene::self->setSceneRect(MyScene::self->itemsBoundingRect());
        MyScene::self->update();
    }
}

bool DrillNode::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::CheckStateRole:
            checkState = value.value<Qt::CheckState>();
            FileHolder::file(m_id)->itemGroup()->setVisible(checkState == Qt::Checked);
            return true;
        default:
            return false;
        }

    default:
        return false;
    }
    return false;
}

Qt::ItemFlags DrillNode::flags(const QModelIndex& index) const
{
    switch (index.column()) {
    case 0:
        return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
    default:
        return Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
    }
}

QVariant DrillNode::data(const QModelIndex& index, int role) const
{
    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::DisplayRole:
            return FileHolder::file(m_id)->shortFileName();
        case Qt::ToolTipRole:
            return FileHolder::file(m_id)->fileName();
        case Qt::CheckStateRole:
            return checkState;
        case Qt::DecorationRole:
            return QIcon::fromTheme("roll");
        case Qt::UserRole:
            return m_id;
        default:
            return QVariant();
        }
    case 1:
        return QVariant();
    default:
        break;
    }

    return QVariant();
}
