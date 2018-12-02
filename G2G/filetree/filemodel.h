#ifndef MODEL_H
#define MODEL_H

#include "abstractnode.h"
#include <QAbstractItemModel>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <file.h>
#include <gcode/drl.h>
#include <gcode/gcode.h>

enum RootNodes {
    NodeGerberFiles,
    NodeDrillFiles,
    NodeToolPath,
};

class FileModel : public QAbstractItemModel {
    Q_OBJECT
    AbstractNode* rootItem;

signals:
    void updateActions();

public:
    static FileModel* self;
    explicit FileModel(QObject* parent = nullptr);
    ~FileModel();

    void addGerberFile(G::File* gerberFile);
    void addDrlFile(DrillFile* drl);
    void addGcode(GCodeFile* group);
    void closeAllFiles();

public:
    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool removeRows(int row, int count, const QModelIndex& parent) override;

    int columnCount(const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent) const override;
};

#endif // MODEL_H
