#include "gcodenode.h"
#include "fileholder.h"

#include <QFileInfo>
#include <QIcon>

GcodeNode::GcodeNode(GCode* group)
    : m_id(FileHolder::addFile(group))
{
}

GcodeNode::~GcodeNode()
{
    FileHolder::deleteFile(m_id);
}

bool GcodeNode::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.column())
        return false;

    switch (role) {
    //        case Qt::DisplayRole:
    //            return gerberFile->fileName;
    //            return true;
    case Qt::CheckStateRole:
        checkState = value.value<Qt::CheckState>();
        FileHolder::file<GCode>(m_id)->itemGroup()->setVisible(checkState == Qt::Checked);
        return true;
    default:
        return false;
    }
}

int GcodeNode::columnCount() const
{
    return 2;
}

int GcodeNode::childCount() const
{
    return 0;
}

Qt::ItemFlags GcodeNode::flags(const QModelIndex&) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
}

QVariant GcodeNode::data(const QModelIndex& index, int role) const
{
    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::DisplayRole:
            return FileHolder::file<GCode>(m_id)->shortFileName();
        case Qt::ToolTipRole:
            return FileHolder::file<GCode>(m_id)->fileName();
        case Qt::CheckStateRole:
            return checkState;
        case Qt::DecorationRole:
            switch (FileHolder::file<GCode>(m_id)->type()) {
            case Profile:
                return QIcon::fromTheme("object-to-path");
            case Pocket:
                return QIcon::fromTheme("stroke-to-path");
            case Drilling:
                return QIcon::fromTheme("roll");
            default:
                return QIcon::fromTheme("roll");
            }
        case Qt::UserRole:
            return QVariant::fromValue(reinterpret_cast<quint64>(FileHolder::file<GCode>(m_id)));
        default:
            return QVariant();
        }
    case 1:
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return QString("Top|Bottom").split('|')[FileHolder::file<GCode>(m_id)->side()];
        default:
            return QVariant();
        }
    default:
        return QVariant();
    }
}
