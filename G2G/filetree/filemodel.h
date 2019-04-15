#ifndef MODEL_H
#define MODEL_H

#include "abstractnode.h"
#include <QAbstractItemModel>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <excellon/exfile.h>
#include <file.h>
#include <gcode/gcode.h>

enum RootNodes {
    NodeGerberFiles,
    NodeDrillFiles,
    NodeToolPath,
    NodeSpecial,
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

    void addGerberFile(Gerber::File* gerberFile);
    void addDrlFile(Excellon::File* drl);
    void addGcode(GCodeFile* group);
    void closeAllFiles();

    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool removeRows(int row, int count, const QModelIndex& parent) override;

    int columnCount(const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent) const override;

private:
    AbstractNode* getItem(const QModelIndex& index) const;
};

#endif // MODEL_H
