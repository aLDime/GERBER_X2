#ifndef MILLING_H
#define MILLING_H

#include "abstractitem.h"
#include <QGraphicsItemGroup>

class Milling : public AbstractItem {
    Qt::CheckState checkState = Qt::Checked;

public:
    Milling(const QString name, QGraphicsItemGroup* group);
    ~Milling();

    // AbstractItem interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    int columnCount(/*const QModelIndex& parent*/) const override;
    int rowCount(/*const QModelIndex& parent*/) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;

private:
    QString name;
    QGraphicsItemGroup *group;
};

#endif // MILLING_H
