#ifndef MYGRAPHICSSCENE_H
#define MYGRAPHICSSCENE_H

#include <QGraphicsItem>
#include <QGraphicsScene>
class Point;

class MyGraphicsScene : public QGraphicsScene {

public:
    explicit MyGraphicsScene(QObject* parent, bool drawPoints = false);
    ~MyGraphicsScene();
    void RenderPdf(QPainter* painter);

    Point* getItemZero() const;
    Point* getItemHome() const;

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
