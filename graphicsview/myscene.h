#ifndef MYGRAPHICSSCENE_H
#define MYGRAPHICSSCENE_H

#include <QGraphicsItem>
#include <QGraphicsScene>
#include "point.h"

class MyScene : public QGraphicsScene {
    friend class Point;

public:
    explicit MyScene(QObject* parent, bool drawPoints = false);
    ~MyScene();
    void RenderPdf();

    Point* getItemZero() const;
    Point* getItemHome() const;
    static MyScene* self;

private:
    bool drawPdf;
    bool drawPoints;
    Point* itemZero = nullptr;
    Point* itemHome = nullptr;

    // QGraphicsScene interface
protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void drawForeground(QPainter* painter, const QRectF& rect) override;
};

#endif // MYGRAPHICSSCENE_H
