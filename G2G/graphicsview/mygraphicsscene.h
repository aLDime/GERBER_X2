#ifndef MYGRAPHICSSCENE_H
#define MYGRAPHICSSCENE_H

#include <QGraphicsItem>
#include <QGraphicsScene>

class Point : public QObject, public QGraphicsItem {
    Q_OBJECT
public:
    Point() {}
    virtual ~Point() {}

    // QGraphicsItem interface
public:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};


class MyGraphicsScene : public QGraphicsScene {

public:
    explicit MyGraphicsScene(QObject* parent, bool drawPoints = false);
    ~MyGraphicsScene();
    void RenderPdf(QPainter* painter);

    QGraphicsPathItem* getItemZero() const;
    QGraphicsPathItem* getItemHome() const;

private:
    bool drawPdf;
    bool drawPoints;
    QGraphicsPathItem* itemZero = nullptr;
    QGraphicsPathItem* itemHome = nullptr;

    // QGraphicsScene interface
protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void drawForeground(QPainter* painter, const QRectF& rect) override;
};

#endif // MYGRAPHICSSCENE_H
