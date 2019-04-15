#include "drillnode.h"
#include "excellon/exfile.h"
#include "filetree/fileholder.h"
#include "gerbernode.h"
#include <QFileInfo>
#include <mainwindow.h>

DrillNode::DrillNode(Excellon::DrillFile* file)
    : m_id(FileHolder::addFile(file))
{
    FileHolder::file(m_id)->itemGroup()->addToTheScene();
}

DrillNode::~DrillNode()
{
    FileHolder::deleteFile(m_id);
    if (Scene::self) {
        Scene::self->setSceneRect(Scene::self->itemsBoundingRect());
        Scene::self->update();
    }
}

bool DrillNode::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::CheckStateRole:
            FileHolder::file(m_id)->itemGroup()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
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
            return FileHolder::file(m_id)->itemGroup()->isVisible() ? Qt::Checked : Qt::Unchecked;
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
