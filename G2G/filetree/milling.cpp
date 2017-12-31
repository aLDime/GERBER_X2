#include "milling.h"

#include <QIcon>

Milling::Milling(const QString name, QGraphicsItem *group)
    : name(name)
    , group(group)
{
}

Milling::~Milling()
{
    delete group;
}

bool Milling::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.column())
        switch (role) {
        //        case Qt::DisplayRole:
        //            return gerberFile->fileName;
        //            return true;
        case Qt::CheckStateRole:
            checkState = value.value<Qt::CheckState>();
            group->setVisible(checkState == Qt::Checked);
            return true;
        default:
            break;
        }
    return false;
}

int Milling::columnCount() const
{
    return 1;
}

int Milling::rowCount() const
{
    return 0;
}

Qt::ItemFlags Milling::flags(const QModelIndex&) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
}

QVariant Milling::data(const QModelIndex& index, int role) const
{
    if (!index.column())
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return name;
        case Qt::EditRole:
            return name;
        case Qt::CheckStateRole:
            return checkState;
        case Qt::DecorationRole:
            return QIcon::fromTheme("object-to-path");
        default:
            break;
        }
    return QVariant();
}
