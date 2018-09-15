#ifndef DRILLITEM_H
#define DRILLITEM_H

#include "graphicsitem.h"
class Drill;
class DrillItem : public GraphicsItem {
public:
    DrillItem(double diameter, Drill* file = nullptr);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;

    double diameter() const;
    void setDiameter(double diameter);

    const Drill* file() const;

    // GraphicsItem interface
    Paths paths() const override;
private:
    double m_diameter;
    const Drill* m_file;


};

#endif // DRILLITEM_H
