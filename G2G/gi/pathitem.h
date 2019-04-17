#ifndef PATHITEM_H
#define PATHITEM_H

#include "graphicsitem.h"

class PathItem : public GraphicsItem {
public:
    PathItem(const Paths& paths);
    PathItem(const Path& path);
    ~PathItem();
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    Paths paths() const override;
};

#endif // PATHITEM_H
