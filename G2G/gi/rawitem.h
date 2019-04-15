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

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    Paths paths() const override;
    QPainterPath shape() const override;

    const Gerber::File* file() const;

private:
    QPolygonF m_polygon;
    const Gerber::File* m_file;
};

#endif // RAWITEM_H
