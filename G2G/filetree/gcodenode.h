#ifndef MILLING_H
#define MILLING_H

#include "abstractnode.h"
#include <QGraphicsItemGroup>
#include <gcode/gcode.h>

class GcodeNode : public AbstractNode {
    Qt::CheckState checkState = Qt::Checked;

public:
    GcodeNode(GCodeFile* gCode);
    ~GcodeNode();

    // AbstractItem interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;

private:
    const int m_id;
};

#endif // MILLING_H
