#ifndef GERBERITEM_H
#define GERBERITEM_H

#include "gerber.h"
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QPen>

using namespace ClipperLib;
namespace G {

enum {
    RawItemType = QGraphicsItem::UserType + 1,
    WorkItemType = QGraphicsItem::UserType + 2
};

class GraphicsItem : public QGraphicsItem {
public:
    GraphicsItem()
        : m_pen(Qt::NoPen)
        , m_brush(Qt::red)
    {
    }
    QBrush brush() const { return m_brush; }
    QPen pen() const { return m_pen; }
    void setBrush(const QBrush& brush) { m_brush = brush; }
    void setPen(const QPen& pen) { m_pen = pen; }

protected:
    QPen m_pen;
    QBrush m_brush;
};

class GraphicsItem;
class ItemGroup : public QList<GraphicsItem*> {
public:
    ~ItemGroup();
    void setVisible(const bool visible);
    bool isVisible() { return m_visible; }
    void addToTheScene(QGraphicsScene* scene);
    QBrush brush() const { return m_brush; }
    QPen pen() const { return m_pen; }
    void setBrush(const QBrush& brush);
    void setPen(const QPen& pen);

private:
    bool m_visible = true;
    QPen m_pen;
    QBrush m_brush;
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

class WorkItem : public GraphicsItem {
public:
    WorkItem(const Paths& paths);
    //~WorkItem() override {}

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    Paths getPaths() const;

private:
    QPainterPath m_shape;
    Paths paths;
    QRectF rect;
};
}
#endif // GERBERITEM_H
