#ifndef TermalPreviewItem_H
#define TermalPreviewItem_H

#include "gi/graphicsitem.h"
#include "tooldatabase/tool.h"
#include <gbrvars.h>
#include <myclipper.h>

class TermalPreviewItem : public QGraphicsItem {
    static QPainterPath drawApetrure(const Gerber::GraphicObject& go, int id);
    static QPainterPath drawPoly(const Gerber::GraphicObject& go);

public:
    //    explicit TermalPreviewItem(const Gerber::GraphicObject& go, int id);
    explicit TermalPreviewItem(const Gerber::GraphicObject& go, Tool& tool);

    ~TermalPreviewItem();

    enum Type {
        Termal = ShtiftType + 1,
    };

    // QGraphicsItem interface
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) override;
    virtual QRectF boundingRect() const override;

    //////////////////////////////////////////
    int type() const override;
    //    void setToolId(int toolId);
    IntPoint pos() const;
    Paths paths() const;
    void redraw();

    double angle() const;
    void setAngle(double angle);

    double tickness() const;
    void setTickness(double tickness);

    int count() const;
    void setCount(int count);

private:
    Tool& tool;
    const Gerber::GraphicObject* const grob = nullptr;

    const QPainterPath m_sourcePath;
    QPainterPath m_toolPath;
    const Type m_type;
    QPen m_pen;
    QBrush m_brush;

    double m_angle = 0.0;
    double m_tickness = 0.5;
    int m_count = 4;
};

#endif // TermalPreviewItem_H
