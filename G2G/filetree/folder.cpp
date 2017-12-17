#include "folder.h"

#include <QIcon>

Folder::Folder(const QString& name)
    : name(name)
{
}

Folder::~Folder()
{
    qDebug() << "~Folder()";
}

QVariant Folder::data(const QModelIndex& index, int role) const
{
    if (!index.column())
        switch (role) {
        case Qt::DisplayRole:
            return name;
        case Qt::DecorationRole:
            return QIcon::fromTheme("folder");
            break;
        default:
            break;
        }

    return QVariant();
}

Qt::ItemFlags Folder::flags(const QModelIndex& index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
}

int Folder::rowCount(const QModelIndex& parent) const
{
    return childItems.size();
}

int Folder::columnCount(const QModelIndex& parent) const
{
    return 1;
}

bool Folder::setData(const QModelIndex& index, const QVariant& value, int role)
{
    return true;
}
