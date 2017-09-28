#ifndef GERBERITEM_H
#define GERBERITEM_H

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QPen>
#include "gerberparser.h"

using namespace ClipperLib;

enum {
    GerberRawItemType = QGraphicsItem::UserType + 1,
    GerberWorkItemType = QGraphicsItem::UserType + 2
};

class GerberItem : public QGraphicsItem {
public:
    GerberItem()
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

class GerberItemGroup : public QList<GerberItem*> {
public:
    void setVisible(const bool visible)
    {
        if (m_visible != visible) {
            m_visible = visible;
            for (QGraphicsItem* item : *this) {
                item->setVisible(m_visible);
            }
        }
    }
    ~GerberItemGroup();
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

class GerberRawItem : public GerberItem {
public:
    GerberRawItem(GERBER_ITEM& item);
    QRectF boundingRect() const override { return rect; }
    QPainterPath shape() const override { return m_shape; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const { return GerberRawItemType; }

private:
    QColor color;
    int m_type = 0;
    QPainterPath path;
    QPainterPath m_shape;
    GerberAperture* aperture = nullptr;
    const double size;
    IMAGE_POLARITY imgPolarity;
    QRectF rect;
};

class GerberWorkItem : public GerberItem {
public:
    GerberWorkItem(const Paths& paths);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const;
    Paths getPaths() const;

private:
    QPainterPath m_shape;
    Paths paths;
    QRectF rect;
};

#endif // GERBERITEM_H
