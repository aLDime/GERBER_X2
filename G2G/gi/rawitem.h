#ifndef RAWITEM_H
#define RAWITEM_H

#include "graphicsitem.h"

namespace Gerber {
class File;
}

class RawItem : /*public QObject, */ public GraphicsItem {
    //Q_OBJECT
public:
    RawItem(const Path& path, Gerber::File* file);

    const Gerber::File* file() const;

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    Paths paths() const override;
    QPainterPath shape() const override;

private:
    QPolygonF m_polygon;
    const Gerber::File* m_file;
    const Path& m_path;
    mutable QPainterPath m_shape;
    mutable double m_scale = 0.0;

    // QGraphicsItem interface
protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
};

#endif // RAWITEM_H
