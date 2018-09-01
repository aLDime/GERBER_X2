#ifndef POINT_H
#define POINT_H

#include "myscene.h"

#include <QObject>
#include <QGraphicsItem>
#include <QBrush>
#include <QPen>

class Point : public QObject, public QGraphicsItem {
public:
    Point(int type);

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QPainterPath shape() const override;

    void setBrush(const QBrush& brush);

private:
    QRectF m_rect;
    QPainterPath m_path;
    QPainterPath m_shape;
    QBrush m_brush;
    QPen m_pen;
    int m_type;
    // QGraphicsItem interface
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
};

#endif // POINT_H
