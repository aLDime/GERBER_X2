#include "filesystemmodel.h"
#include <QDir>
#include <algorithm>
#include <QFileIconProvider>
#include <QDateTime>
#include <QDebug>

FilesystemModel::FilesystemModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    fetchRoot();
}

struct FilesystemModel::NodeInfo {
    NodeInfo()
        : parent(nullptr)
        , mapped(false)
    {
    }

    NodeInfo(const Tool& tool, NodeInfo* parent = 0)
        : tool(tool)
        , parent(parent)
        , mapped(false)
    {
    }

    //    bool operator==(const NodeInfo& another) const
    //    {
    //        bool r = this->toolInfo == another.toolInfo;
    //        Q_ASSERT(!r || this->parent == another.parent);
    //        Q_ASSERT(!r || this->mapped == another.mapped);
    //        Q_ASSERT(!r || this->children == another.children);
    //        return r;
    //    }
    bool insertChildren(int position, int count, int /*columns*/)
    {
        if (position < 0 || position > children.size())
            return false;
        for (int row = 0; row < count; ++row) {
            children.insert(position, NodeInfo(Tool(), this));
        }
        return true;
    }

    NodeInfo* child(int number)
    {
        if (children.size() > number)
            return &children[number];
        else
            return nullptr;
        /*.value(number);*/
    }

    bool isRoot() const { return parent == nullptr; }
    Tool tool;
    QVector<NodeInfo> children;
    NodeInfo* parent;

    bool mapped;
};

FilesystemModel::~FilesystemModel()
{
}

QModelIndex FilesystemModel::index(int row, int column, const QModelIndex& parent) const
{
    //    if (!hasIndex(row, column, parent)) {
    //        return QModelIndex();
    //    }

    //    if (!parent.isValid()) {
    //        Q_ASSERT(_nodes.size() > row);
    //        return createIndex(row, column, const_cast<NodeInfo*>(&_nodes[row]));
    //    }

    //    NodeInfo* parentInfo = static_cast<NodeInfo*>(parent.internalPointer());
    //    Q_ASSERT(parentInfo != 0);
    //    Q_ASSERT(parentInfo->mapped);
    //    Q_ASSERT(parentInfo->children.size() > row);
    //    return createIndex(row, column, &parentInfo->children[row]);

    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();
    //! [5]

    //! [6]
    NodeInfo* parentItem = getItem(parent);

    NodeInfo* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex FilesystemModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    NodeInfo* childInfo = static_cast<NodeInfo*>(child.internalPointer());
    Q_ASSERT(childInfo != 0);
    NodeInfo* parentInfo = childInfo->parent;
    if (parentInfo != 0) {
        return createIndex(findRow(parentInfo), RamificationColumn, parentInfo);
    }
    else {
        return QModelIndex();
    }
}

int FilesystemModel::findRow(const NodeInfo* nodeInfo) const
{
    Q_ASSERT(nodeInfo != 0);
    const NodeInfoList& parentInfoChildren = nodeInfo->parent != 0 ? nodeInfo->parent->children : _nodes;
    //    NodeInfoList::const_iterator position = std::find(parentInfoChildren.begin(), parentInfoChildren.end(), nodeInfo);
    //    Q_ASSERT(position != parentInfoChildren.end());
    //    return std::distance(parentInfoChildren.begin(), position);
    return 0;
}

FilesystemModel::NodeInfo* FilesystemModel::getItem(const QModelIndex& index) const
{
    if (index.isValid()) {
        NodeInfo* item = static_cast<NodeInfo*>(index.internalPointer());
        if (item)
            return item;
    }
    return const_cast<NodeInfo*>(&_nodes.first());
}

int FilesystemModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        return _nodes.size();
    }
    const NodeInfo* parentInfo = static_cast<const NodeInfo*>(parent.internalPointer());
    Q_ASSERT(parentInfo != 0);

    return parentInfo->children.size();
}

bool FilesystemModel::hasChildren(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        const NodeInfo* parentInfo = static_cast<const NodeInfo*>(parent.internalPointer());
        Q_ASSERT(parentInfo != 0);
        if (!parentInfo->mapped) {
            return true; //QDir(parentInfo->toolInfo.absoluteFilePath()).count() > 0;
        }
    }
    return QAbstractItemModel::hasChildren(parent);
}

bool FilesystemModel::insertRows(int position, int rows, const QModelIndex& parent)
{
    qDebug() << "insertRows";
    NodeInfo* parentItem = getItem(parent);
    bool success;
    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, ColumnCount);
    endInsertRows();
    return success;
}

int FilesystemModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return ColumnCount;
}

QVariant FilesystemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const NodeInfo* nodeInfo = static_cast<NodeInfo*>(index.internalPointer());
    const Tool& toolInfo = nodeInfo->tool;
    Q_ASSERT(nodeInfo != 0);

    switch (index.column()) {
    case NameColumn:
        return toolInfo.name;
    case NoteColumn:
        return toolInfo.note;
        break;
    default:
        break;
    }
    return QVariant("123");
}

//QVariant FilesystemModel::nameData(const Tool& toolInfo, int role) const
//{
//    switch (role) {
//    case Qt::EditRole:
//        return toolInfo.Name;
//    case Qt::DisplayRole:
//        if (toolInfo.isRoot()) {
//            return toolInfo.absoluteFilePath();
//        }
//        else if (toolInfo.isDir()) {
//            return toolInfo.Name;
//        }
//        else {
//            return toolInfo.completeBaseName();
//        }
//    case Qt::DecorationRole:
//    //return _metaProvider->icon(toolInfo);
//    default:
//        return QVariant();
//    }
//    Q_UNREACHABLE();
//}

bool FilesystemModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid()) {
        return false;
    }
    if (role != Qt::EditRole) {
        return false;
    }
    if (index.column() > ColumnCount) {
        return false;
    }
    NodeInfo* toolInfo = static_cast<NodeInfo*>(index.internalPointer());

    switch (index.column()) {
    case NameColumn:
        toolInfo->tool.name = value.toString();
        break;
    case NoteColumn:
        toolInfo->tool.note = value.toString();
        break;
    default:
        break;
    }

    //    QString newName = value.toString();
    //    if (newName.contains('/') || newName.contains(QDir::separator())) {
    //        return false;
    //    }
    //    NodeInfo* nodeInfo = static_cast<NodeInfo*>(index.internalPointer());
    //    QString fullNewName = nodeInfo->tool.absoluteDir().path() + "/" + newName;
    //    QString fullOldName = nodeInfo->tool.absoluteFilePath();
    //    qDebug() << fullOldName << fullNewName;
    //    bool renamed = QFile::rename(fullOldName, fullNewName);
    //    qDebug() << renamed;
    //    if (renamed) {
    //        nodeInfo->tool = Tool(fullNewName);
    //        emit dataChanged(index, index.sibling(index.row(), ColumnCount));
    //    }
    return true;
}

QVariant FilesystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    const QStringList headers = { "Name", "Note" };
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section < headers.size()) {
        return headers[section];
    }
    return QVariant();
}

bool FilesystemModel::canFetchMore(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        return false;
    }

    const NodeInfo* parentInfo = static_cast<const NodeInfo*>(parent.internalPointer());
    Q_ASSERT(parentInfo != 0);
    return !parentInfo->mapped;
}

//void FilesystemModel::fetchMore(const QModelIndex& parent)
//{
//    Q_ASSERT(parent.isValid());
//    NodeInfo* parentInfo = static_cast<NodeInfo*>(parent.internalPointer());
//    Q_ASSERT(parentInfo != 0);
//    Q_ASSERT(!parentInfo->mapped);

//    const Tool& toolInfo = parentInfo->tool;
//    Q_ASSERT(toolInfo.isDir());

//    QDir dir = QDir(toolInfo.absoluteFilePath());
//    ToolList children = dir.entryInfoList(QStringList(), QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Name);

//    int insrtCnt = children.size() - 1;
//    if (insrtCnt < 0) {
//        insrtCnt = 0;
//    }
//    beginInsertRows(parent, 0, insrtCnt);
//    parentInfo->children.reserve(children.size());
//    for (const Tool& entry : children) {
//        NodeInfo nodeInfo(entry, parentInfo);
//        nodeInfo.mapped = !entry.isDir();
//        parentInfo->children.push_back(std::move(nodeInfo));
//    }
//    parentInfo->mapped = true;
//    endInsertRows();
//}

void FilesystemModel::fetchRoot()
{
    //    const ToolList drives = QDir::drives();
    _nodes.append(NodeInfo());
    //    std::copy(drives.begin(), drives.end(), std::back_inserter(_nodes));
}

Qt::ItemFlags FilesystemModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (index.isValid() && index.column() == NameColumn) {
        const NodeInfo* nodeInfo = static_cast<const NodeInfo*>(index.internalPointer());
        if (!nodeInfo->isRoot()) {
            flags |= Qt::ItemIsEditable;
        }
    }
    return flags;
}
