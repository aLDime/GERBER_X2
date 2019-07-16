#ifndef GERBERITEM_H
#define GERBERITEM_H

#include <QGraphicsItem>
#include <QPen>

#include <myclipper.h>

using namespace ClipperLib;

enum GiType{
    GerberItemType = QGraphicsItem::UserType,
    BridgeType,
    DrillItemType,
    PathItemType,
    PointHomeType,
    PointZeroType,
    RawItemType,
    RulerType,
    PinType,
};

class AbstractFile;

namespace Gerber {
class File;
}
namespace Excellon {
class File;
}

class GraphicsItem : public QGraphicsItem {
    friend class Gerber::File;
    friend class Excellon::File;

public:
    explicit GraphicsItem(AbstractFile* file = nullptr);
    ~GraphicsItem();

    QBrush brush() const;
    QPen pen() const;
    void setBrush(const QBrush& brush);
    void setPen(const QPen& pen);
    virtual Paths paths() const = 0;

    //    void setItemGroup(ItemGroup* itemGroup);
    //    ItemGroup* parentItemGroup() const;
    //    QPointF center() const;

    void setPenColor(QColor& penColor);
    void setBrushColor(QColor& brushColor);

    const AbstractFile* file() const;
    template <typename T>
    const T* typedFile() const { return dynamic_cast<const T* const>(m_file); }

    int id() const;

protected:
    int m_id = -1;
    QPen m_pen;
    QBrush m_brush;
    QColor* m_penColor = nullptr;
    QColor* m_brushColor = nullptr;

    QPainterPath m_shape;
    QRectF m_rect;
    const AbstractFile* m_file;

    //private:
    //    ItemGroup* m_ig = nullptr;
};

#endif // GERBERITEM_H
