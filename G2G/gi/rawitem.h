#ifndef RAWITEM_H
#define RAWITEM_H

#include "graphicsitem.h"

namespace G {
class File;
}

class RawItem : /*public QObject, */ public GraphicsItem {
    //Q_OBJECT
public:
    RawItem(const Path& path, G::File* file);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    Paths paths() const override;
    QPainterPath shape() const override;

    const G::File* file() const;

private:
    QPolygonF m_polygon;
    const G::File* m_file;
};

//class RawItem : public GraphicsItem {
//public:
//    RawItem(GraphicObject& item);
//    QRectF boundingRect() const override { return rect; }
//    QPainterPath shape() const override { return m_shape; }
//    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
//    int type() const { return RawItemType; }

//private:
//    QColor color;
//    int m_type = 0;
//    QPainterPath path;
//    QPainterPath m_shape;
//    GerberAperture* aperture = nullptr;
//    const double size;
//    IMAGE_POLARITY imgPolarity;
//    QRectF rect;
//};

#endif // RAWITEM_H
