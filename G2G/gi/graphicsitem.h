#ifndef GERBERITEM_H
#define GERBERITEM_H

#include <QGraphicsItem>
#include <QPen>

#include <myclipper.h>
using namespace ClipperLib;

enum {
    GERBER_ITEM = QGraphicsItem::UserType + 1,
    DRILL_ITEM,
    PATH_ITEM,
    RAW_ITEM,
    POINT_HOME,
    POINT_ZERO,
    POINT_SHTIFT,
};

class GraphicsItem : public QGraphicsItem {
public:
    GraphicsItem();
    QBrush brush() const;
    QPen pen() const;
    void setBrush(const QBrush& brush);
    void setPen(const QPen& pen);
    virtual Paths paths() const = 0;

protected:
    QPen m_pen;
    QBrush m_brush;

    QPainterPath m_shape;
    mutable Paths m_paths;
    QRectF m_rect;
};

#endif // GERBERITEM_H
