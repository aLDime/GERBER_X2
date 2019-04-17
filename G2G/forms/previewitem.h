#ifndef PREVIEWITEM_H
#define PREVIEWITEM_H

#include "gi/graphicsitem.h"
#include <exvars.h>
#include <gbrvars.h>
#include <myclipper.h>

class PreviewItem : public QGraphicsItem {
    static QPainterPath drawApetrure(const Gerber::GraphicObject& go, int id);
    static QPainterPath drawDrill(const Excellon::Hole& hole);
    static QPainterPath drawSlot(const Excellon::Hole& hole);

public:
    explicit PreviewItem(const Gerber::GraphicObject& go, int id);
    explicit PreviewItem(const Excellon::Hole& hole);

    ~PreviewItem();

    enum Type {
        Slot = ShtiftType + 1,
        Drill,
        Apetrure
    };

    // QGraphicsItem interface
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) override;
    virtual QRectF boundingRect() const override;

    //////////////////////////////////////////
    int type() const override;
    double sourceDiameter() const;
    int toolId() const;
    void setToolId(int toolId);
    void setSelected(bool value);
    IntPoint pos() const;
    Paths paths() const;
    bool fit(double depth);

private:
    const int id = 0;
    const Gerber::GraphicObject* const grob = nullptr;
    const Excellon::Hole* const hole = nullptr;

    QPainterPath m_sourcePath;
    QPainterPath m_toolPath;

    const double m_sourceDiameter;
    int m_toolId = -1;
    const Type m_type;

    QPen m_pen;
    QBrush m_brush;

    bool isSelected = false;
};

#endif // PREVIEWITEM_H
