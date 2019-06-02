#include "drillnode.h"
#include "project.h"
#include "gerbernode.h"
#include <QFileInfo>
#include <exfile.h>
#include <mainwindow.h>

DrillNode::DrillNode(Excellon::File* file)
    : m_id(Project::addFile(file))
{
    Project::file(m_id)->itemGroup()->addToTheScene();
}

DrillNode::~DrillNode()
{
    qDebug("~DrillNode()");
    Project::deleteFile(m_id);
    if (Scene::self) {
        Scene::self->setSceneRect(Scene::self->itemsBoundingRect());
        Scene::self->update();
    }
}

bool DrillNode::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (index.column()) {
    case Name:
        switch (role) {
        case Qt::CheckStateRole:
            Project::file(m_id)->itemGroup()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
            return true;
        default:
            return false;
        }
    case Layer:
        switch (role) {
        case Qt::EditRole:
            Project::file(m_id)->setSide(static_cast<Side>(value.toBool()));
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
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    switch (index.column()) {
    case Name:
        return itemFlag | Qt::ItemIsUserCheckable;
    case Layer:
        return itemFlag | Qt::ItemIsEditable;
    default:
        return itemFlag;
    }
}

QVariant DrillNode::data(const QModelIndex& index, int role) const
{
    switch (index.column()) {
    case Name:
        switch (role) {
        case Qt::DisplayRole:
            return Project::file(m_id)->shortFileName();
        case Qt::ToolTipRole:
            return Project::file(m_id)->shortFileName() + "\n"
                + Project::file(m_id)->fileName();
        case Qt::CheckStateRole:
            return Project::file(m_id)->itemGroup()->isVisible() ? Qt::Checked : Qt::Unchecked;
        case Qt::DecorationRole:
            return Icon(PathDrillIcon);
        case Qt::UserRole:
            return m_id;
        default:
            return QVariant();
        }
    case Layer:
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return tr("Top|Bottom").split('|')[Project::file(m_id)->side()];
        case Qt::EditRole:
            return static_cast<bool>(Project::file(m_id)->side());
        case Qt::UserRole:
            return m_id;
        default:
            return QVariant();
        }
    default:
        return QVariant();
    }
    return QVariant();
}
