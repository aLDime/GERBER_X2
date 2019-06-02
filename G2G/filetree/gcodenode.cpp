#include "gcodenode.h"
#include "project.h"
#include "icons.h"

#include <QFileInfo>

GcodeNode::GcodeNode(GCodeFile* gCode)
    : m_id(Project::addFile(gCode))
{
    gCode->itemGroup()->addToTheScene();
}

GcodeNode::~GcodeNode()
{
    Project::deleteFile(m_id);
}

bool GcodeNode::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::CheckStateRole:
            Project::file(m_id)->itemGroup()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
            return true;
        default:
            return false;
        }
    case 1:
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

Qt::ItemFlags GcodeNode::flags(const QModelIndex& index) const
{
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    switch (index.column()) {
    case 0:
        return itemFlag | Qt::ItemIsUserCheckable;
    case 1: {
        if (Project::file(m_id)->shortFileName().contains(".tap"))
            return itemFlag;
        else
            return itemFlag | Qt::ItemIsEditable;
    }
    default:
        return itemFlag;
    }
}

QVariant GcodeNode::data(const QModelIndex& index, int role) const
{
    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::DisplayRole: {
            //            if (Project::file(m_id)->shortFileName().contains(".tap"))
            return Project::file(m_id)->shortFileName();
            //            else
            //                return Project::file(m_id)->shortFileName() + QStringList({ "(Top)", "(Bot)" })[Project::file(m_id)->side()];
        }
        case Qt::ToolTipRole:
            return Project::file(m_id)->shortFileName() + "\n" + Project::file(m_id)->fileName();
        case Qt::CheckStateRole:
            return Project::file(m_id)->itemGroup()->isVisible() ? Qt::Checked : Qt::Unchecked;
        case Qt::DecorationRole:
            return Icon(Project::file<GCodeFile>(m_id)->gtype());
        case Qt::UserRole:
            return m_id;
        default:
            return QVariant();
        }
    case 1:
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return QStringList({ QObject::tr("Top"), QObject::tr("Bottom") })[Project::file(m_id)->side()];
        case Qt::EditRole:
            return static_cast<bool>(Project::file(m_id)->side());
        default:
            return QVariant();
        }
    default:
        return QVariant();
    }
}
