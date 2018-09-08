#ifndef MILLING_H
#define MILLING_H

#include "abstractitem.h"
#include <QGraphicsItemGroup>
#include <gcode/gcode.h>

class GcodeItem : public AbstractItem {
    Qt::CheckState checkState = Qt::Checked;

public:
    GcodeItem(GCode* group);
    ~GcodeItem();

    // AbstractItem interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    int columnCount(/*const QModelIndex& parent*/) const override;
    int childCount(/*const QModelIndex& parent*/) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    static QMap<int, GCode*> gCode;

private:
    const int m_id;
};

#endif // MILLING_H
