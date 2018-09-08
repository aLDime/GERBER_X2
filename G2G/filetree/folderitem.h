#ifndef FOLDER_H
#define FOLDER_H

#include "abstractitem.h"

class FolderItem : public AbstractItem {
    QString name;

public:
    FolderItem(const QString& name);
    ~FolderItem();
    // AbstractItem interface
public:
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    int childCount(/*const QModelIndex& parent*/) const override;
    int columnCount(/*const QModelIndex& parent*/) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

private:
    Qt::CheckState checkState = Qt::Checked;
};
#endif // FOLDER_H
