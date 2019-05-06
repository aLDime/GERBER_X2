#include "termalmodel.h"

QIcon TermalModel::repaint(QColor color, const QIcon& icon) const
{
    QImage image(icon.pixmap(24, 24).toImage());
    for (int x = 0; x < 24; ++x)
        for (int y = 0; y < 24; ++y) {
            color.setAlpha(image.pixelColor(x, y).alpha());
            image.setPixelColor(x, y, color);
        }
    return QIcon(QPixmap::fromImage(image));
}

TermalModel::TermalModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    rootItem = new TermalNode(QIcon(), "");
}

TermalModel::~TermalModel()
{
    delete rootItem;
}

TermalNode* TermalModel::appendRow(const QIcon& icon, const QString& name)
{
    m_data.append(new TermalNode(icon, name));
    rootItem->append(m_data.last());
    return m_data.last();
}

int TermalModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
        return 0;
    return getItem(parent)->childCount();
}

int TermalModel::columnCount(const QModelIndex& parent) const
{
    return 5;
}

QModelIndex TermalModel::index(int row, int column, const QModelIndex& parent) const
{
    TermalNode* childItem = getItem(parent)->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex TermalModel::parent(const QModelIndex& index) const
{

    if (!index.isValid())
        return QModelIndex();

    TermalNode* parentItem = getItem(index)->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

QVariant TermalModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();
    TermalNode* item = getItem(index);
    return item->data(index, role);
}

bool TermalModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
        return false;
    const bool result = getItem(index)->setData(index, value, role);
    return result;
}

QVariant TermalModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    static const QVector<QString> horizontalLabel{ "     Name", "Pos (X:Y)", "Angle", "Tickness", "Count" };
    switch (role) {
    case Qt::DisplayRole:
        if (orientation == Qt::Horizontal)
            return horizontalLabel[section];
        else
            return section + 1;
    default:
        return QVariant();
    }
    //    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    //        switch (section) {
    //        case 0:
    //            return tr("Name");
    //        case 1:
    //            return tr("Side");
    //        default:
    //            return QString("");
    //        }
    //    return QVariant();
}

Qt::ItemFlags TermalModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    TermalNode* item = getItem(index);
    return item->flags(index);
}

bool TermalModel::removeRows(int row, int count, const QModelIndex& parent)
{
    TermalNode* item = nullptr;
    if (parent.isValid())
        item = static_cast<TermalNode*>(parent.internalPointer());
    else
        return false;
    beginRemoveRows(parent, row, row + count - 1);
    while (count--)
        item->remove(row);
    endRemoveRows();
    resetInternalData();
    return true;
}

TermalNode* TermalModel::getItem(const QModelIndex& index) const
{
    if (index.isValid()) {
        TermalNode* item = static_cast<TermalNode*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

//void TermalModel::appendRow(const QString& name, const QIcon& icon, int id) { m_data.append(Row(name, icon)); }

//int TermalModel::rowCount(const QModelIndex& /*parent*/) const
//{
//    return m_data.size();
//}

//int TermalModel::columnCount(const QModelIndex& /*parent*/) const
//{
//    return 4;
//}

//QVariant TermalModel::data(const QModelIndex& index, int role) const
//{
//    int row = index.row();
//    if (!index.column())
//        switch (role) {
//        case Qt::DisplayRole:
//            //            if (m_data[row].isSlot)
//            //                return QString(m_data[row].name).replace(tr("Tool"), tr("Slot"));
//            //            else
//            return m_data[row].name;
//        case Qt::DecorationRole: {
//            //            if (m_data[index.row()].toolId > -1 && m_data[row].isSlot) {
//            //                return repaint(QColor(255, 0, 0), m_data[row].icon);
//            //            } else if (m_data[index.row()].toolId > -1) {
//            //                return m_data[row].icon;
//            //            } else if (m_data[row].isSlot) {
//            //                return repaint(QColor(255, 100, 100), m_data[row].icon);
//            //            } else {
//            return repaint(QColor(100, 100, 100), m_data[row].icon);
//            //            }
//        }
//            //        case Qt::UserRole:
//            //            return m_data[row].apToolId;
//        case Qt::CheckStateRole:
//            return m_data[row].create ? Qt::Checked : Qt::Unchecked;
//        default:
//            break;
//        }
//    else {
//        //        if (m_data[row].toolId == -1)
//        //            switch (role) {
//        //            case Qt::DisplayRole:
//        //                return tr("Abra Cadabra");
//        //            case Qt::TextAlignmentRole:
//        //                return Qt::AlignCenter;
//        //                //            case Qt::UserRole:
//        //                //                return m_data[row].toolId;
//        //            default:
//        //                break;
//        //            }
//        //        else
//        //            switch (role) {
//        //            case Qt::DisplayRole:
//        //                return ToolHolder::tools[m_data[row].toolId].name;
//        //            case Qt::DecorationRole:
//        //                return ToolHolder::tools[m_data[row].toolId].icon();
//        //            case Qt::UserRole:
//        //                return m_data[row].toolId;
//        //            default:
//        //                break;
//        //            }
//    }
//    return QVariant();
//}

//bool TermalModel::setData(const QModelIndex& index, const QVariant& value, int role)
//{
//    return false;
//}

//QVariant TermalModel::headerData(int section, Qt::Orientation orientation, int role) const
//{
//    static const QVector<QString> horizontalLabel{ "Name", "Angle", "Tickness", "Count" };
//    switch (role) {
//    case Qt::DisplayRole:
//        if (orientation == Qt::Horizontal)
//            return horizontalLabel[section];
//        else
//            return section + 1;
//    default:
//        return QVariant();
//    }
//}

//Qt::ItemFlags TermalModel::flags(const QModelIndex& index) const
//{
//    if (!index.column())
//        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
//    else
//        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
//}


