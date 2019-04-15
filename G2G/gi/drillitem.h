#ifndef DRILLITEM_H
#define DRILLITEM_H

#include "graphicsitem.h"

namespace Excellon {
class DrillFile;
class Hole;
}

class DrillItem : public GraphicsItem {
public:
    DrillItem(Excellon::Hole* hole);
    DrillItem(double diameter);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    bool isSlot() { return !m_paths.isEmpty(); }

    double diameter() const;
    void setDiameter(double diameter);
    void updateHole();

    const Excellon::DrillFile* file() const;

    // GraphicsItem interface
    Paths paths() const override;

private:
    void create();
    Excellon::Hole* const m_hole = nullptr;
    double m_diameter = 0.0;
};

#endif // DRILLITEM_H
