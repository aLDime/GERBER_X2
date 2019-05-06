#ifndef TERMALMODEL_H
#define TERMALMODEL_H

#include "termalpreviewitem.h"

#include <QAbstractItemModel>
#include <QIcon>
#include <myclipper.h>

//class TermalPreviewItem;

class TermalNode {
    TermalNode(const TermalNode&) = delete;
    TermalNode& operator=(const TermalNode&) = delete;

public:
    TermalNode(const QIcon& icon, const QString& name, double angle, double tickness, int count, const IntPoint& pos, TermalPreviewItem* item)
        : container(false)
        , icon(icon)
        , name(name)
        , m_angle(angle)
        , m_tickness(tickness)
        , m_count(count)
        , m_pos(pos)
        , m_item(item)
    {
    }
    TermalNode(const QIcon& icon, const QString& name)
        : container(true)
        , icon(icon)
        , name(name)
        , m_angle(0)
        , m_tickness(0.5)
        , m_count(4)
    {
    }

    ~TermalNode()
    {
        childItems.clear();
    }

    TermalNode* child(int row) const
    {
        return childItems.value(row).data();
    }

    TermalNode* parentItem()
    {
        return m_parentItem;
    }

    int childCount() const
    {
        return childItems.count();
    }

    int row() const
    {
        if (m_parentItem)
            for (int i = 0, size = m_parentItem->childItems.size(); i < size; ++i)
                if (m_parentItem->childItems[i].data() == this)
                    return i;
        return 0;
    }

    void append(TermalNode* item)
    {
        item->m_parentItem = this;
        childItems.append(QSharedPointer<TermalNode>(item));
    }
    void remove(int row) { childItems.removeAt(row); }

    bool setData(const QModelIndex& index, const QVariant& value, int role)
    {
        switch (role) {
        case Qt::EditRole:
            if (container) {
                switch (index.column()) {
                case 0:
                case 1:
                    return false;
                case 2:
                    for (QSharedPointer<TermalNode> item : childItems) {
                        item->setData(index, value, role);
                    }
                    return m_angle = value.toDouble();
                    return true;
                case 3:
                    for (QSharedPointer<TermalNode> item : childItems) {
                        item->setData(index, value, role);
                    }
                    return m_tickness = value.toDouble();
                    return true;
                case 4:
                    for (QSharedPointer<TermalNode> item : childItems) {
                        item->setData(index, value, role);
                    }
                    return m_count = value.toInt();
                    return true;
                }
                return false;
            } else {
                switch (index.column()) {
                case 0:
                case 1:
                    return false;
                case 2:
                    m_item->setAngle(value.toDouble());
                    return true;
                case 3:
                    m_item->setTickness(value.toDouble());
                    return true;
                case 4:
                    m_item->setCount(value.toInt());
                    return true;
                }
            }
            return false;
        case Qt::CheckStateRole:
            if (!index.column()) {
                m_checkState = value.toBool() ? Qt::Checked : Qt::Unchecked;
                if (m_item) {
                    if (value.toBool()) {
                        m_item->setFlag(QGraphicsItem::ItemIsSelectable, value.toBool());
                        m_item->setSelected(selected);
                    } else {
                        selected = m_item->isSelected();
                        m_item->setFlag(QGraphicsItem::ItemIsSelectable, value.toBool());
                    }
                }
                for (QSharedPointer<TermalNode> item : childItems) {
                    item->setData(index, value, role);
                }
                return true;
            }
            return false;
        default:
            break;
        }
        return false;
    }

    Qt::ItemFlags flags(const QModelIndex& index) const
    {
        Qt::ItemFlags flags = Qt::ItemIsEnabled;
        if (!index.column())
            flags |= Qt::ItemIsUserCheckable;
        if (index.column() > 1 /*&& !container*/)
            flags |= Qt::ItemIsEditable;
        if (!container)
            flags |= Qt::ItemNeverHasChildren;
        if (m_checkState == Qt::Checked)
            flags |= Qt::ItemIsSelectable;
        return flags;
    }

    QVariant data(const QModelIndex& index, int role) const
    {
        switch (role) {
        case Qt::EditRole:
        case Qt::DisplayRole:
            if (container) {
                switch (index.column()) {
                case 0:
                    return name;
                case 1:
                    return QString("%1 : %2").arg(m_pos.X * dScale).arg(m_pos.Y * dScale).replace('.', ',');
                case 2:
                    return m_angle;
                case 3:
                    return m_tickness;
                case 4:
                    return m_count;
                }
            } else {
                switch (index.column()) {
                case 0:
                    return name;
                case 1:
                    return QString("%1 : %2").arg(m_pos.X * dScale).arg(m_pos.Y * dScale).replace('.', ',');
                case 2:
                    return m_item->angle();
                case 3:
                    return m_item->tickness();
                case 4:
                    return m_item->count();
                }
            }
            return QVariant();
        case Qt::DecorationRole:
            if (!index.column())
                return icon;
            return QVariant();
        case Qt::CheckStateRole:
            if (!index.column())
                return m_checkState;
            return QVariant();
        case Qt::TextAlignmentRole:
            //            if (index.column())
            //                return Qt::AlignCenter;
            return QVariant();
        default:
            break;
        }
        return QVariant();
    }

    double angle() const { return m_angle; }
    double tickness() const { return m_tickness; }
    int count() const { return m_count; }
    IntPoint pos() const { return m_pos; }
    TermalPreviewItem* item() const { return m_item; }
    bool create() const { return m_checkState == Qt::Checked && !container; }

private:
    bool container = false;
    const QIcon icon;
    const QString name;
    double m_angle;
    double m_tickness;
    int m_count;
    const IntPoint m_pos;

    TermalPreviewItem* m_item = nullptr;
    bool selected = false;

    TermalNode* m_parentItem = nullptr;
    QList<QSharedPointer<TermalNode>> childItems;
    Qt::CheckState m_checkState = Qt::Checked;
};

class TermalModel : public QAbstractItemModel {
    Q_OBJECT

    //    typedef struct Row {
    //        Row(const QString& name, const QIcon& icon)
    //            : name{ name }
    //            , icon{ icon }
    //        {
    //        }
    //        QString name;
    //        QIcon icon;
    //        bool create = false;
    //    } Row;
    //    QList<Row> m_data;

    QIcon repaint(QColor color, const QIcon& icon) const;

    TermalNode* rootItem = nullptr;
    QList<TermalNode*> m_data;

public:
    explicit TermalModel(QObject* parent = nullptr);
    ~TermalModel();

    TermalNode* appendRow(const QIcon& icon, const QString& name);

    //signals:
    //public slots:

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex& parent) const override;
    virtual int columnCount(const QModelIndex& parent) const override;

    //    virtual QVariant data(const QModelIndex& index, int role) const override;
    //    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    //    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    //    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool removeRows(int row, int count, const QModelIndex& parent) override;

private:
    TermalNode* getItem(const QModelIndex& index) const;
};

#endif // TERMALMODEL_H
