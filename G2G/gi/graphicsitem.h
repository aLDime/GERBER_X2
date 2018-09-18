#ifndef GERBERITEM_H
#define GERBERITEM_H

#include <QGraphicsItem>
#include <QPen>

#include <myclipper.h>
using namespace ClipperLib;

enum {
    GerberItemType = QGraphicsItem::UserType + 1,
    DrillItemType,
    PathItemType,
    RAW_ITEM_Type,
    PointHomeType,
    PointZeroType,
    ShtiftType,
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
