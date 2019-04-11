#ifndef FOLDER_H
#define FOLDER_H

#include "abstractnode.h"

class FolderNode : public AbstractNode {
    QString name;
    Qt::CheckState m_checkState = Qt::Checked;

public:
    FolderNode(const QString& name);
    ~FolderNode();

    // AbstractItem interface
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
};
#endif // FOLDER_H
