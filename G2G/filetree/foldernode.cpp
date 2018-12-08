#include "foldernode.h"
#include "gerbernode.h"

#include <QIcon>

FolderNode::FolderNode(const QString& name)
    : name(name)
{
}

FolderNode::~FolderNode()
{
}

QVariant FolderNode::data(const QModelIndex& index, int role) const
{
    if (index.column())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        return name;
    case Qt::DecorationRole:
        return QIcon::fromTheme("folder");
        //    case Qt::CheckStateRole:
        //        return checkState;
    default:
        return QVariant();
    }
}

Qt::ItemFlags FolderNode::flags(const QModelIndex& /*index*/) const
{
    return Qt::ItemIsEnabled /*| Qt::ItemIsUserCheckable|*/;
}

bool FolderNode::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.column())
        return false;

    switch (role) {
    case Qt::CheckStateRole:
        checkState = value.value<Qt::CheckState>();
        return true;
    default:
        return false;
    }
}
