#include "filemodel.h"
#include "drillnode.h"
#include "foldernode.h"
#include "gcodenode.h"
#include "gerbernode.h"
#include <QApplication>
#include <QDebug>
#include <QMimeData>
#include <QStandardItem>

FileModel* FileModel::self = nullptr;

FileModel::FileModel(QObject* parent)
    : QAbstractItemModel(parent)
    , rootItem(new FolderNode("rootItem"))
{
    self = this;
    rootItem->add(new FolderNode("Gerber Files"));
    rootItem->add(new FolderNode("Excellon"));
    rootItem->add(new FolderNode("Tool Paths"));
}

FileModel::~FileModel()
{
    delete rootItem;
    self = nullptr;
}

void FileModel::addGerberFile(G::File* gerberFile)
{
    AbstractNode* item{ rootItem->child(NodeGerberFiles) };
    QModelIndex index = createIndex(0, 0, item);
    int rowCount = item->childCount();
    beginInsertRows(index, rowCount, rowCount);
    item->add(new GerberNode(gerberFile));
    endInsertRows();
}

void FileModel::addDrlFile(DrillFile* drl)
{
    if (!drl)
        return;
    AbstractNode* item{ rootItem->child(NodeDrillFiles) };
    QModelIndex index = createIndex(0, 0, item);
    int rowCount = item->childCount();
    beginInsertRows(index, rowCount, rowCount);
    item->add(new DrillNode(drl));
    endInsertRows();
}

void FileModel::addGcode(GCodeFile* group)
{
    AbstractNode* item{ rootItem->child(NodeToolPath) };
    QModelIndex index = createIndex(0, 0, item);
    int rowCount = item->childCount();
    beginInsertRows(index, rowCount, rowCount);
    item->add(new GcodeNode(group));
    endInsertRows();
}

void FileModel::closeAllFiles()
{
    AbstractNode* item{ rootItem->child(NodeGerberFiles) };
    QModelIndex index = createIndex(0, 0, item);
    int rowCount = item->childCount();
    if (rowCount) {
        beginRemoveRows(index, 0, rowCount - 1);
        for (int i = 0; i < rowCount; ++i) {
            item->remove(0);
        }
        endRemoveRows();
    }

    item = rootItem->child(NodeDrillFiles);
    index = createIndex(0, 0, item);
    rowCount = item->childCount();
    if (rowCount) {
        beginRemoveRows(index, 0, rowCount - 1);
        for (int i = 0; i < rowCount; ++i) {
            item->remove(0);
        }
        endRemoveRows();
    }

    item = rootItem->child(NodeToolPath);
    index = createIndex(0, 0, item);
    rowCount = item->childCount();
    if (rowCount) {
        beginRemoveRows(index, 0, rowCount - 1);
        for (int i = 0; i < rowCount; ++i) {
            item->remove(0);
        }
        endRemoveRows();
    }
}

QModelIndex FileModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    AbstractNode* parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<AbstractNode*>(parent.internalPointer());

    AbstractNode* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex FileModel::parent(const QModelIndex& child) const
{
    if (!child.isValid())
        return QModelIndex();

    AbstractNode* parentItem = static_cast<AbstractNode*>(child.internalPointer())->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

QVariant FileModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();
    AbstractNode* item = static_cast<AbstractNode*>(index.internalPointer());
    return item->data(index, role);
}

bool FileModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
        return false;
    AbstractNode* item = static_cast<AbstractNode*>(index.internalPointer());
    return item->setData(index, value, role);
}

QVariant FileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
        switch (section) {
        case 0:
            return tr("Name");
        case 1:
            return tr("Side");
        default:
            return QString("");
        }
    //    QString("Name|Note").split('|')[section];
    return QVariant();
}

Qt::ItemFlags FileModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    AbstractNode* item = static_cast<AbstractNode*>(index.internalPointer());
    return item->flags(index);
}

bool FileModel::removeRows(int row, int count, const QModelIndex& parent)
{
    //qDebug() << parent << parent.data();
    //qDebug() << row << count;
    AbstractNode* item = nullptr;
    if (parent.isValid())
        item = static_cast<AbstractNode*>(parent.internalPointer());
    else
        item = rootItem;

    beginRemoveRows(parent, row, row + count - 1);
    for (int r = row; r < row + count; ++r)
        item->remove(r);
    endRemoveRows();
    return true;
}

int FileModel::columnCount(const QModelIndex& /*parent*/) const
{
    //    if (parent.isValid())
    //        return static_cast<AbstractItem*>(parent.internalPointer())->columnCount();
    //    else
    //        return rootItem->columnCount();

    return 2;
}

int FileModel::rowCount(const QModelIndex& parent) const
{
    AbstractNode* parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<AbstractNode*>(parent.internalPointer());

    return parentItem->childCount();
}
