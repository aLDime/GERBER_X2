#ifndef SLOTITEM_H
#define SLOTITEM_H

#include "graphicsitem.h"

class DrillFile;

class SlotItem : public GraphicsItem {
    //Q_OBJECT
public:
    SlotItem(const Path& path, double size, DrillFile* file);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    Paths paths() const override;
    QPainterPath shape() const override;

    const DrillFile* file() const;

private:
    QPolygonF m_polygon;
    const DrillFile* m_file;
    const double m_size;
};
#endif // SLOTITEM_H
