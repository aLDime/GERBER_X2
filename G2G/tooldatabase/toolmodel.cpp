#include "toolmodel.h"
#include <QApplication>
#include <QDebug>
#include <QMimeData>
#include <QStandardItem>

ToolModel::ToolModel(QObject* parent)
    : QAbstractItemModel(parent)
    , rootItem(new ToolItem(Tool()))
{
    importTools();
}

ToolModel::~ToolModel()
{
    exportTools();
    delete rootItem;
}

bool ToolModel::insertRows(int row, int count, const QModelIndex& parent)
{
    qDebug() << "insertRows" << row << count << data(parent).toString();
    beginInsertRows(parent, row, row + count - 1);
    ToolItem* parentItem = static_cast<ToolItem*>(parent.internalPointer());
    if (!parentItem)
        parentItem = rootItem;
    if (parentItem->childCount() > row)
        parentItem->insertChild(row, new ToolItem(Tool()));
    else
        parentItem->addChild(new ToolItem(Tool()));
    endInsertRows();
    return true;
}

bool ToolModel::removeRows(int row, int count, const QModelIndex& parent)
{
    qDebug() << "removeRows" << row << count << parent << createIndex(row, 0, rootItem);
    beginRemoveRows(parent, row, row + count - 1);
    ToolItem* parentItem = static_cast<ToolItem*>(parent.internalPointer());
    if (!parentItem)
        parentItem = rootItem;
    parentItem->removeChild(row);
    endRemoveRows();
    return true;
}

int ToolModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 2;
}

int ToolModel::rowCount(const QModelIndex& parent) const
{
    ToolItem* parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<ToolItem*>(parent.internalPointer());

    return parentItem->childCount();
}

QModelIndex ToolModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ToolItem* parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<ToolItem*>(parent.internalPointer());

    ToolItem* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex ToolModel::parent(const QModelIndex& child) const
{
    if (!child.isValid())
        return QModelIndex();

    ToolItem* childItem = static_cast<ToolItem*>(child.internalPointer());
    ToolItem* parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

Qt::ItemFlags ToolModel::flags(const QModelIndex& index) const
{
    ToolItem* item = static_cast<ToolItem*>(index.internalPointer());
    if (!item)
        item = rootItem;
    return item->flags(index);
}

bool ToolModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    ToolItem* item = static_cast<ToolItem*>(index.internalPointer());
    if (!item)
        item = rootItem;
    if (item->setData(index, value, role)) {
        return true;
    }
    return false;
}

QVariant ToolModel::data(const QModelIndex& index, int role) const
{
    ToolItem* item = static_cast<ToolItem*>(index.internalPointer());
    if (!item)
        item = rootItem;
    return item->data(index, role);
}

QVariant ToolModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
        return QString("column 1|column 2|column 3").split('|')[section];
    return QVariant();
}

QStringList ToolModel::mimeTypes() const
{
    QStringList types;
    types << myModelMimeType();
    return types;
}

QMimeData* ToolModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;
    int noCopy = -1;
    for (const QModelIndex& index : indexes) {
        if (noCopy != index.row()) {
            noCopy = index.row();
            if (index.isValid()) {
                encodedData.append(QString().setNum((quint64)index.internalPointer()).toLocal8Bit());
                encodedData.append("|");
            }
        }
    }
    mimeData->setData(myModelMimeType(), encodedData);
    return mimeData;
}

bool ToolModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    qDebug() << "dropMimeData" << row << column;

    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat(myModelMimeType()))
        return false;

    if (column > 0)
        return false;

    int beginRow;

    if (row != -1)
        beginRow = row;
    else if (parent.isValid())
        beginRow = parent.row();
    else
        beginRow = rowCount(QModelIndex());

    QString encodedData = data->data(myModelMimeType());
    QList<QString> list = encodedData.split('|', QString::SkipEmptyParts);

    for (QString& item : list) {
        ToolItem* copyItem = reinterpret_cast<ToolItem*>(item.toLongLong());
        ToolItem* parentItem = static_cast<ToolItem*>(parent.internalPointer());
        if (copyItem) {
            if (!parentItem)
                parentItem = rootItem;
            insertRows(beginRow, list.size(), parent);
            if (parentItem->childCount() > beginRow)
                parentItem->setChild(beginRow, new ToolItem(*copyItem));
            else
                parentItem->setChild(parentItem->childCount() - 1, new ToolItem(*copyItem));
        }
        ++beginRow;
    }
    return true;
}
