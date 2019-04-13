#include "drillmodel.h"
#include "tooldatabase/tool.h"

DrillModel::DrillModel(int type, QObject* parent)
    : QAbstractTableModel(parent)
    , m_type(type)
{
}

void DrillModel::appendRow(const QString& name, const QIcon& icon, int id)
{
    m_data.append(Row(name, icon, id));
}

void DrillModel::setToolId(int row, int id)
{
    m_data[row].id[1] = id;
    //    QModelIndex index(createIndex(row, 1));
    //    dataChanged(index, index);
}

int DrillModel::toolId(int row) { return m_data[row].id[1]; }

void DrillModel::setSlot(int row, bool slot) { m_data[row].isSlot = slot; }

bool DrillModel::isSlot(int row) { return m_data[row].isSlot; }

void DrillModel::setApertureId(int row, int id)
{
    m_data[row].id[0] = id;
    //    QModelIndex index(createIndex(row, 0));
    //    dataChanged(index, index);
}

int DrillModel::apertureId(int row) { return m_data[row].id[0]; }

int DrillModel::rowCount(const QModelIndex& /*parent*/) const { return m_data.size(); }

int DrillModel::columnCount(const QModelIndex& /*parent*/) const { return 2; }

QVariant DrillModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    if (!index.column())
        switch (role) {
        case Qt::DisplayRole:
            return m_data[row].name[0];
        case Qt::DecorationRole:
            return m_data[row].icon[0];
        case Qt::UserRole:
            return m_data[row].id[0];
        default:
            break;
        }
    else {
        if (m_data[row].id[1] == -1)
            switch (role) {
            case Qt::DisplayRole:
                return "Select Tool";
            case Qt::TextAlignmentRole:
                return Qt::AlignCenter;
            case Qt::UserRole:
                return m_data[row].id[index.column()];
            default:
                break;
            }
        else
            switch (role) {
            case Qt::DisplayRole:
                return ToolHolder::tools[m_data[row].id[1]].name;
            case Qt::DecorationRole:
                return ToolHolder::tools[m_data[row].id[1]].icon();
            case Qt::UserRole:
                return m_data[row].id[index.column()];
            default:
                break;
            }
    }
    return QVariant();
}

QVariant DrillModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                switch (m_type) {
                case tAperture:
                    return "Aperture";
                case tTool:
                    return "Tool";
                }
            case 1:
                return "Tool";
            }

        } else {
            switch (m_type) {
            case tAperture:
                return QString("D%1").arg(m_data[section].id[0]);
            case tTool:
                return QString("T%1").arg(m_data[section].id[0]);
            }
        }
        return QVariant();
    case Qt::TextAlignmentRole:
        return Qt::AlignCenter;
    default:
        return QVariant();
    }
}

Qt::ItemFlags DrillModel::flags(const QModelIndex& index) const
{
    if (index.column())
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    return Qt::ItemIsEnabled;
}
