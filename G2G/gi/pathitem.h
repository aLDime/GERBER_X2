#ifndef PATHITEM_H
#define PATHITEM_H

#include "graphicsitem.h"

class PathItem : public GraphicsItem {
public:
    PathItem(const Path& m_path);
    //~WorkItem() override {}

    QRectF boundingRect() const override;
    //    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;

    // GraphicsItem interface
    Paths paths() const override;
};

#endif // PATHITEM_H
