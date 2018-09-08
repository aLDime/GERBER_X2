#include "gcodeitem.h"

#include <QIcon>
QMap<int, GCode*> GcodeItem::gCode;

GcodeItem::GcodeItem(GCode* group)
    : m_id(gCode.size() ? gCode.lastKey() + 1 : 0)
{
    gCode[m_id] = group;
}

GcodeItem::~GcodeItem()
{
    if (gCode.contains(m_id) && gCode[m_id])
        delete gCode.take(m_id);
}

bool GcodeItem::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.column())
        return false;

    switch (role) {
    //        case Qt::DisplayRole:
    //            return gerberFile->fileName;
    //            return true;
    case Qt::CheckStateRole:
        checkState = value.value<Qt::CheckState>();
        gCode[m_id]->setVisible(checkState == Qt::Checked);
        return true;
    default:
        return false;
    }
}

int GcodeItem::columnCount() const
{
    return 1;
}

int GcodeItem::childCount() const
{
    return 0;
}

Qt::ItemFlags GcodeItem::flags(const QModelIndex&) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
}

QVariant GcodeItem::data(const QModelIndex& index, int role) const
{
    if (index.column())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
        return gCode[m_id]->name();
    case Qt::EditRole:
        return gCode[m_id]->name();
    case Qt::CheckStateRole:
        return checkState;
    case Qt::DecorationRole:
        switch (gCode[m_id]->type) {
        case PROFILE:
            return QIcon::fromTheme("object-to-path");
        case POCKET:
            return QIcon::fromTheme("stroke-to-path");
        case DRILLING:
            return QIcon::fromTheme("roll");
        default:
            return QIcon::fromTheme("roll");
        }
    case Qt::UserRole:
        return QVariant::fromValue(reinterpret_cast<quint64>(gCode[m_id]));
    default:
        return QVariant();
    }
}
