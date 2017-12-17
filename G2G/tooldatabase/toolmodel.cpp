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
    //qDebug() << "insertRows" << row << count << data(parent).toString();
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
    //qDebug() << "removeRows" << row << count << parent << createIndex(row, 0, rootItem);
    beginRemoveRows(parent, row, row + count - 1);
    ToolItem* parentItem = static_cast<ToolItem*>(parent.internalPointer());
    if (!parentItem)
        parentItem = rootItem;
    parentItem->removeChild(row);
    endRemoveRows();
    return true;
}

bool ToolModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild)
{
    return false;
    beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild);
    ToolItem* srcItem = static_cast<ToolItem*>(sourceParent.internalPointer());
    ToolItem* dstItem = static_cast<ToolItem*>(destinationParent.internalPointer());
    if (!srcItem)
        srcItem = rootItem;
    if (!dstItem)
        dstItem = rootItem;
    for (int r = 0; r < count; ++r) {
        dstItem->insertChild(destinationChild + r, srcItem->takeChild(sourceRow));
    }
    endMoveRows();
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
        return QString("Name|Note").split('|')[section];
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
    //    QMimeData* mimeData = new QMimeData();
    //    QByteArray encodedData;
    //    int noCopy = -1;
    //    for (const QModelIndex& index : indexes) {
    //        if (noCopy != index.row()) {
    //            noCopy = index.row();
    //            ToolItem* item = static_cast<ToolItem*>(index.parent().internalPointer());
    //            if (!item)
    //                item = rootItem;
    //            if (index.isValid()) {
    //                encodedData.append(QString("%1,%2").arg(index.row()).arg((quint64)item /*index.internalPointer()*/).toLocal8Bit());
    //                encodedData.append("|");
    //            }
    //        }
    //    }
    //    mimeData->setData(myModelMimeType(), encodedData);
    //    return mimeData;
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

    //    for (QString& item : list) {
    //        QList<QString> d = item.split(',', QString::SkipEmptyParts);
    //        if (d.size() < 2)
    //            return false;
    //        int srcRow = d.at(0).toInt();
    //        ToolItem* ti = reinterpret_cast<ToolItem*>(d.at(1).toLongLong());
    //        QModelIndex index = createIndex(srcRow, 0, ti);
    //        moveRows(index, srcRow, 1, parent, parent.row() > -1 ? parent.row() : 0);
    //    }

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

Qt::DropActions ToolModel::supportedDragActions() const { return Qt::MoveAction | Qt::TargetMoveAction; }

Qt::DropActions ToolModel::supportedDropActions() const { return Qt::MoveAction | Qt::TargetMoveAction; }

QString ToolModel::myModelMimeType() { return QStringLiteral("application/ToolItem"); }

void ToolModel::exportTools()
{
    QList<QString> lines;
    ToolItem* item;
    QList<ToolItem*> stack;
    QList<int> row;

    stack.push_back(rootItem);
    row.append(0);

    while (stack.size()) {
        if (stack.last()->childCount() && row.last()) {
            stack.pop_back();
            row.pop_back();
            if (!stack.size())
                break;
            ++row.last();
        }
        else if (stack.last() == rootItem && stack.last()->childCount() == 0) {
            break;
        }
        while (stack.last()->childCount() > row.last()) {
            item = stack.last()->child(row.last());
            QString str(row.size() - 1, '\t');
            str += (item->getTool().name.isEmpty() ? " " : item->getTool().name) + "\t";
            str += (item->getTool().note.isEmpty() ? " " : item->getTool().note) + "\t";
            str += item->getTool().toHex();
            lines << str;
            if (item->childCount()) {
                stack.push_back(item);
                row.push_back(0);
                break;
            }
            ++row.last();
        }
    }

    QFile file("D:/PRO/QT_PROJECTS/ToolDatabase.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out.setCodec("UTF-8");
    for (QString line : lines) {
        out << line << endl;
    }
    file.close();
}

void ToolModel::importTools()
{
    QFile file("D:/PRO/QT_PROJECTS/ToolDatabase.txt");
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, tr("Unable to open file"), file.errorString());
        return;
    }
    QList<QString> lines;
    QTextStream in(&file);
    in.setCodec("UTF-8");
    while (!in.atEnd()) {
        lines << in.readLine();
    }

    file.close();
    QList<ToolItem*> parentsStack;
    QList<int> nestingStack;
    parentsStack << rootItem;
    nestingStack << 0;
    int number = 0;
    while (number < lines.count()) {
        int nesting = 0;
        while (nesting < lines[number].length()) {
            if (lines[number].at(nesting) != '\t')
                break;
            ++nesting;
        }

        // Read the column data from the rest of the line.
        QStringList toolData = lines[number].split("\t", QString::SkipEmptyParts);
        if (!toolData.isEmpty()) {
            if (toolData.count() < 3) {
                QMessageBox::information(0, "", tr("Tool Database is corupted!"));
                return;
            }

            if (nesting > nestingStack.last()) {
                // The last child of the current parent is now the new parent unless the current parent has no children.
                if (parentsStack.last()->childCount() > 0) {
                    parentsStack.push_back(parentsStack.last()->child(parentsStack.last()->childCount() - 1));
                    nestingStack.push_back(nesting);
                }
            }
            else {
                while (nesting < nestingStack.last() && parentsStack.count() > 0) {
                    parentsStack.pop_back();
                    nestingStack.pop_back();
                }
            }

            // Append a new item to the current parent's list of children.
            ToolItem* parent = parentsStack.last();
            parent->insertChild(parent->childCount(), new ToolItem(Tool(toolData)));
        }

        ++number;
    }
}
