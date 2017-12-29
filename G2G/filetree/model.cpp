#include "Model.h"
#include "Model.h"
#include <QApplication>
#include <QDebug>
#include <QMimeData>
#include <QStandardItem>

Model* Model::model = nullptr;

Model::Model(QObject* parent)
    : QAbstractItemModel(parent)
    , rootItem(new FolderItem("rootItem"))
{
    importTools();
    rootItem->add(new FolderItem("Файлы"));
    rootItem->add(new FolderItem("Фрезеровки"));
    rootItem->add(new FolderItem("Сверловки"));
    rootItem->add(new FolderItem("Штифты"));
    model = this;
}

Model::~Model()
{
    exportTools();
    delete rootItem;
}

bool Model::insertRows(int row, int count, const QModelIndex& parent)
{
    //qDebug() << "insertRows" << row << count << data(parent).toString();
    beginInsertRows(parent, row, row + count - 1);
    AbstractItem* parentItem = static_cast<AbstractItem*>(parent.internalPointer());
    if (!parentItem)
        parentItem = rootItem;
    if (parentItem->rowCount(/*parent*/) > row)
        parentItem->insert(row, new /*AbstractItem*/ FolderItem(""));
    else
        parentItem->add(new /*AbstractItem*/ FolderItem(""));
    endInsertRows();
    return true;
}

bool Model::removeRows(int row, int count, const QModelIndex& parent)
{
    //qDebug() << "removeRows" << row << count << parent << createIndex(row, 0, rootItem);
    beginRemoveRows(parent, row, row + count - 1);
    AbstractItem* parentItem = static_cast<AbstractItem*>(parent.internalPointer());
    if (!parentItem)
        parentItem = rootItem;
    for (int r = 0; r < count; ++r) {
        parentItem->remove(row);
    }
    endRemoveRows();
    return true;
}

int Model::columnCount(const QModelIndex& parent) const
{
    return 2;
    AbstractItem* parentItem;
    //    if (parent.column() > 0)
    //        return 0;
    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<AbstractItem*>(parent.internalPointer());
    return parentItem->columnCount(/*parent*/);
}

int Model::rowCount(const QModelIndex& parent) const
{
    AbstractItem* parentItem;
    if (parent.column() > 0)
        return 0;
    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<AbstractItem*>(parent.internalPointer());

    return parentItem->rowCount(/*parent*/);
}

QModelIndex Model::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    AbstractItem* parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<AbstractItem*>(parent.internalPointer());

    AbstractItem* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex Model::parent(const QModelIndex& child) const
{
    if (!child.isValid())
        return QModelIndex();

    AbstractItem* childItem = static_cast<AbstractItem*>(child.internalPointer());
    AbstractItem* parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

Qt::ItemFlags Model::flags(const QModelIndex& index) const
{

    AbstractItem* item = static_cast<AbstractItem*>(index.internalPointer());
    //    if (dynamic_cast<Folders*>(item))
    //        qDebug() << typeid(dynamic_cast<Folders*>(item)).name();
    //    if (dynamic_cast<AbstractItem*>(item))
    //        qDebug() << typeid(dynamic_cast<AbstractItem*>(item)).name();

    if (!item)
        item = rootItem;
    return item->flags(index);
}

bool Model::setData(const QModelIndex& index, const QVariant& value, int role)
{
    AbstractItem* item = static_cast<AbstractItem*>(index.internalPointer());
    if (!item)
        item = rootItem;
    return item->setData(index, value, role);
}

QVariant Model::data(const QModelIndex& index, int role) const
{
    AbstractItem* item = static_cast<AbstractItem*>(index.internalPointer());
    if (!item)
        item = rootItem;
    return item->data(index, role);
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
        return QString("Name|Note").split('|')[section];
    return QVariant();
}

void Model::addGerberFile(G::GFile* gerberFile)
{
    QModelIndex index = createIndex(0, 0, rootItem->child(FILES_F));
    int rowCount = rootItem->child(FILES_F)->rowCount();
    insertRows(rowCount, 1, index);
    rootItem->child(FILES_F)->set(rowCount, new FileItem(gerberFile));
}

void Model::addMilling(const QString name, QGraphicsItemGroup* group)
{
    QModelIndex index = createIndex(0, 0, rootItem->child(MILLING_F));
    int rowCount = rootItem->child(MILLING_F)->rowCount();
    insertRows(rowCount, 1, index);
    rootItem->child(MILLING_F)->set(rowCount, new Milling(name, group));
    emit updateActions();
}

void Model::exportTools()
{
    //    QList<QString> lines;
    //    Item* item;
    //    QList<Item*> stack;
    //    QList<int> row;
    //    stack.push_back(rootItem);
    //    row.append(0);
    //    while (stack.size()) {
    //        if (stack.last()->childCount() && row.last()) {
    //            stack.pop_back();
    //            row.pop_back();
    //            if (!stack.size())
    //                break;
    //            ++row.last();
    //        }
    //        else if (stack.last() == rootItem && stack.last()->childCount() == 0) {
    //            break;
    //        }
    //        while (stack.last()->childCount() > row.last()) {
    //            item = stack.last()->child(row.last());
    //            QString str(row.size() - 1, '\t');
    //            str += (item->getTool().name.isEmpty() ? " " : item->getTool().name) + "\t";
    //            str += (item->getTool().note.isEmpty() ? " " : item->getTool().note) + "\t";
    //            str += item->getTool().toHex();
    //            lines << str;
    //            if (item->childCount()) {
    //                stack.push_back(item);
    //                row.push_back(0);
    //                break;
    //            }
    //            ++row.last();
    //        }
    //    }
    //    QFile file("ToolDatabase.txt");
    //    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    //        return;
    //    QTextStream out(&file);
    //    out.setCodec("UTF-8");
    //    for (QString line : lines) {
    //        out << line << endl;
    //    }
    //    file.close();
}

void Model::importTools()
{
    //    QFile file("ToolDatabase.txt");
    //    if (!file.open(QIODevice::ReadOnly)) {
    //        QMessageBox::information(0, tr("Unable to open file"), file.errorString());
    //        return;
    //    }
    //    QList<QString> lines;
    //    QTextStream in(&file);
    //    in.setCodec("UTF-8");
    //    while (!in.atEnd()) {
    //        lines << in.readLine();
    //    }
    //    file.close();
    //    QList<Item*> parentsStack;
    //    QList<int> nestingStack;
    //    parentsStack << rootItem;
    //    nestingStack << 0;
    //    int number = 0;
    //    while (number < lines.count()) {
    //        int nesting = 0;
    //        while (nesting < lines[number].length()) {
    //            if (lines[number].at(nesting) != '\t')
    //                break;
    //            ++nesting;
    //        }
    //        // Read the column data from the rest of the line.
    //        QStringList toolData = lines[number].split("\t", QString::SkipEmptyParts);
    //        if (!toolData.isEmpty()) {
    //            if (toolData.count() < 3) {
    //                QMessageBox::information(0, "", tr("Tool Database is corupted!"));
    //                return;
    //            }
    //            if (nesting > nestingStack.last()) {
    //                // The last child of the current parent is now the new parent unless the current parent has no children.
    //                if (parentsStack.last()->childCount() > 0) {
    //                    parentsStack.push_back(parentsStack.last()->child(parentsStack.last()->childCount() - 1));
    //                    nestingStack.push_back(nesting);
    //                }
    //            }
    //            else {
    //                while (nesting < nestingStack.last() && parentsStack.count() > 0) {
    //                    parentsStack.pop_back();
    //                    nestingStack.pop_back();
    //                }
    //            }
    //            // Append a new item to the current parent's list of children.
    //            Item* parent = parentsStack.last();
    //            parent->insertChild(parent->childCount(), new Item(Tool(toolData)));
    //        }
    //        ++number;
    //    }
}
