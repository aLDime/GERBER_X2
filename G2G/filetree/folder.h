#ifndef FOLDER_H
#define FOLDER_H

#include "abstractitem.h"

class Folder : public AbstractItem {
    QString name;

public:
    Folder(const QString& name);
    ~Folder();
    // AbstractItem interface
public:
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
};
#endif // FOLDER_H
