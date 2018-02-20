#ifndef POINT_H
#define POINT_H

#include "mygraphicsscene.h"

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

    void setBrush(const QBrush& value);

private:
    QRectF rect;
    QPainterPath path;
    QPainterPath shape_;
    QBrush brush;
    QPen pen;
    int type;
    // QGraphicsItem interface
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
};

#endif // POINT_H
