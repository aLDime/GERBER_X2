#ifndef MILLING_H
#define MILLING_H

#include "abstractnode.h"
#include <QGraphicsItemGroup>
#include <gcode/gcfile.h>

class GcodeNode : public AbstractNode {

public:
    GcodeNode(GCode::File* gCode);
    ~GcodeNode();

    // AbstractItem interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;

private:
    const int m_id;
};

#endif // MILLING_H
