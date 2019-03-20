#ifndef BRIDGEITEM_H
#define BRIDGEITEM_H

#include "graphicsitem.h"
#include <QObject>

class MyGraphicsView;

class BridgeItem : public QObject, public GraphicsItem {
    Q_OBJECT
public:
    explicit BridgeItem(double& lenght, BridgeItem*& ptr, double& size);
    ~BridgeItem() { m_ptr = nullptr; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    void setNewPos(const QPointF& pos);
    // GraphicsItem interface
    Paths paths() const override;

    bool ok() const;
    double lenght() const;
    double angle() const;
    void update();

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    BridgeItem*& m_ptr;
    QPainterPath m_path;
    QPointF calculate(const QPointF& pos);
    bool m_ok = false;
    double& m_lenght;
    double& m_size;
    double m_angle = 0.0;
};

#endif // BRIDGEITEM_H
