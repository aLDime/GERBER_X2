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
    RawItemType,
    PointHomeType,
    PointZeroType,
    ShtiftType,
    BridgeType,
    RulerType
};

class ItemGroup;
class GraphicsItem : public QGraphicsItem {
public:
    GraphicsItem();
    ~GraphicsItem();
    QBrush brush() const;
    QPen pen() const;
    void setBrush(const QBrush& brush);
    void setPen(const QPen& pen);
    virtual Paths paths() const = 0;
    void setItemGroup(ItemGroup* itemGroup);
    ItemGroup* parentItemGroup() const;
    QPointF center() const;

    void setPenColor(QColor& penColor);
    void setBrushColor(QColor& brushColor);

protected:
    QPen m_pen;
    QBrush m_brush;
    QColor* m_penColor = nullptr;
    QColor* m_brushColor = nullptr;

    QPainterPath m_shape;
    mutable Paths m_paths;
    QRectF m_rect;

private:
    ItemGroup* m_ig = nullptr;
};

#endif // GERBERITEM_H
