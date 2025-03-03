#ifndef MYMODEL_H
#define MYMODEL_H

#include <QAbstractItemModel>

class ToolItem;

class ToolModel : public QAbstractItemModel {
    Q_OBJECT
    friend class ToolTreeView;
    ToolItem* rootItem;

public:
    explicit ToolModel(QObject* parent = nullptr);
    ~ToolModel();

    // QAbstractItemModel interface

    bool insertRows(int row, int count, const QModelIndex& parent) override;
    bool removeRows(int row, int count, const QModelIndex& parent) override;
    int columnCount(const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;
    static inline QString myModelMimeType();
    bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild) override;

private:
    void exportTools();
    void importTools();
    ToolItem* getItem(const QModelIndex& index) const
    {
        if (index.isValid()) {
            ToolItem* item = static_cast<ToolItem*>(index.internalPointer());
            if (item)
                return item;
        }
        return rootItem;
    }
};

#endif // MYMODEL_H
