#include "folderitem.h"

#include <QIcon>

FolderItem::FolderItem(const QString& name)
    : name(name)
{
}

FolderItem::~FolderItem()
{
    //qDebug() << "~Folder()";
}

QVariant FolderItem::data(const QModelIndex& index, int role) const
{
    if (!index.column()) {
        switch (role) {
        case Qt::DisplayRole:
            return name;
        case Qt::DecorationRole:
            return QIcon::fromTheme("folder");
        default:
            break;
        }
    }
    return QVariant();
}

Qt::ItemFlags FolderItem::flags(const QModelIndex& /*index*/) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
}

int FolderItem::rowCount() const
{
    return childItems.size();
}

int FolderItem::columnCount() const
{
    return 1;
}

bool FolderItem::setData(const QModelIndex& /*index*/, const QVariant& /*value*/, int /*role*/)
{
    return false;
}
