#ifndef FOLDER_H
#define FOLDER_H

#include "abstractnode.h"

class FolderNode : public AbstractNode {
    QString name;

public:
    FolderNode(const QString& name);
    ~FolderNode();

    // AbstractItem interface
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

private:
    Qt::CheckState checkState = Qt::Checked;
};
#endif // FOLDER_H
