#ifndef MODEL_H
#define MODEL_H

#include "abstractitem.h"
#include "gerber/gerber.h"
#include "gerber/file.h"
#include <QAbstractItemModel>
#include <QDebug>
#include <QFile>
#include <QMessageBox>

enum {
    NODE_FILES,
    NODE_MILLING,
    NODE_DRILL,
    NODE_PINS,
};

class Model : public QAbstractItemModel {
    Q_OBJECT
    AbstractItem* rootItem;

signals:
    void updateActions();

public:
    static Model* model;
    explicit Model(QObject* parent = nullptr);
    ~Model();

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

    void addGerberFile(G::File *gerberFile);
    void addMilling(const QString name, QGraphicsItem *group);
    //    QStringList mimeTypes() const override
    //    {
    //        QStringList types;
    //        types << myModelMimeType();
    //        return types;
    //    }
    //    QMimeData* mimeData(const QModelIndexList& indexes) const override
    //    {
    //        //    QMimeData* mimeData = new QMimeData();
    //        //    QByteArray encodedData;
    //        //    int noCopy = -1;
    //        //    for (const QModelIndex& index : indexes) {
    //        //        if (noCopy != index.row()) {
    //        //            noCopy = index.row();
    //        //            Item* item = static_cast<Item*>(index.parent().internalPointer());
    //        //            if (!item)
    //        //                item = rootItem;
    //        //            if (index.isValid()) {
    //        //                encodedData.append(QString("%1,%2").arg(index.row()).arg((quint64)item /*index.internalPointer()*/).toLocal8Bit());
    //        //                encodedData.append("|");
    //        //            }
    //        //        }
    //        //    }
    //        //    mimeData->setData(myModelMimeType(), encodedData);
    //        //    return mimeData;
    //        QMimeData* mimeData = new QMimeData();
    //        QByteArray encodedData;
    //        int noCopy = -1;
    //        for (const QModelIndex& index : indexes) {
    //            if (noCopy != index.row()) {
    //                noCopy = index.row();
    //                if (index.isValid()) {
    //                    encodedData.append(QString().setNum((quint64)index.internalPointer()).toLocal8Bit());
    //                    encodedData.append("|");
    //                }
    //            }
    //        }
    //        mimeData->setData(myModelMimeType(), encodedData);
    //        return mimeData;
    //    }
    //    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override
    //    {
    //        if (action == Qt::IgnoreAction)
    //            return true;

    //        if (!data->hasFormat(myModelMimeType()))
    //            return false;

    //        if (column > 0)
    //            return false;

    //        int beginRow;

    //        if (row != -1)
    //            beginRow = row;
    //        else if (parent.isValid())
    //            beginRow = parent.row();
    //        else
    //            beginRow = rowCount(QModelIndex());

    //        QString encodedData = data->data(myModelMimeType());
    //        QList<QString> list = encodedData.split('|', QString::SkipEmptyParts);

    //        //    for (QString& item : list) {
    //        //        QList<QString> d = item.split(',', QString::SkipEmptyParts);
    //        //        if (d.size() < 2)
    //        //            return false;
    //        //        int srcRow = d.at(0).toInt();
    //        //        Item* ti = reinterpret_cast<Item*>(d.at(1).toLongLong());
    //        //        QModelIndex index = createIndex(srcRow, 0, ti);
    //        //        moveRows(index, srcRow, 1, parent, parent.row() > -1 ? parent.row() : 0);
    //        //    }

    //        for (QString& item : list) {
    //            AbstractItem* copyItem = reinterpret_cast<AbstractItem*>(item.toLongLong());
    //            AbstractItem* parentItem = static_cast<AbstractItem*>(parent.internalPointer());
    //            if (copyItem) {
    //                if (!parentItem)
    //                    parentItem = rootItem;
    //                insertRows(beginRow, list.size(), parent);
    //                if (parentItem->childCount() > beginRow)
    //                    parentItem->set(beginRow, new AbstractItem(*copyItem));
    //                else
    //                    parentItem->set(parentItem->childCount() - 1, new AbstractItem(*copyItem));
    //            }
    //            ++beginRow;
    //        }
    //        return true;
    //    }
    //    Qt::DropActions supportedDragActions() const override { return Qt::MoveAction | Qt::TargetMoveAction; }
    //    Qt::DropActions supportedDropActions() const override { return Qt::MoveAction | Qt::TargetMoveAction; }
    //    static inline QString myModelMimeType() { return QStringLiteral("application/Item"); }

private:
    void exportTools();
    void importTools();
};

#endif // MODEL_H
