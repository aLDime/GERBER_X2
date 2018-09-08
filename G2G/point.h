#ifndef POINT_H
#define POINT_H

#include "myscene.h"

#include <QBrush>
#include <QGraphicsItem>
#include <QObject>
#include <QPen>

class Point : public QObject, public QGraphicsItem {
    Q_OBJECT

public:
    Point(int type);

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QPainterPath shape() const override;

    void setBrush(const QBrush& brush);
    void setPosition(const QPointF& position);
    void resetPosition();

    bool bounding() const;
    void setBounding(bool bounding);

signals:
    void posChanged(const QPointF& pos);

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

class Shtift : public QObject, public QGraphicsItem {
    Q_OBJECT

public:
    Shtift(int num);
    ~Shtift();
    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QPainterPath shape() const override;

    void setBrush(const QBrush& brush);
    void setPosition(const QPointF& position);

    bool bounding() const;
    void setBounding(bool bounding);

    void setShtifts(const QVector<Shtift*>& shtifts);

signals:
    void posChanged(const QPointF& pos);

private:
    QRectF m_rect;
    QPainterPath m_path;
    QPainterPath m_shape;
    QBrush m_brush;
    QPen m_pen;
    QVector<Shtift*> m_shtifts;
    QPointF m_lastPos;

    // QGraphicsItem interface
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
};

#endif // POINT_H
