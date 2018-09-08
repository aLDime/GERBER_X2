#include "folderitem.h"
#include "gerberitem.h"

#include <QIcon>

FolderItem::FolderItem(const QString& name)
    : name(name)
{
}

FolderItem::~FolderItem()
{
}

QVariant FolderItem::data(const QModelIndex& index, int role) const
{
    if (index.column())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        return name;
    case Qt::DecorationRole:
        return QIcon::fromTheme("folder");
    case Qt::CheckStateRole:
        //        return checkState;
    default:
        return QVariant();
    }
}

Qt::ItemFlags FolderItem::flags(const QModelIndex& /*index*/) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
}

int FolderItem::childCount() const
{
    return childItems.size();
}

int FolderItem::columnCount() const
{
    return 1;
}

bool FolderItem::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.column())
        return false;
    switch (role) {
    case Qt::CheckStateRole:
        checkState = value.value<Qt::CheckState>();
        //        for (G::File* f : GerberItem::gFiles) {
        //            if()
        //        }
        return true;
    default:
        return false;
    }
}
