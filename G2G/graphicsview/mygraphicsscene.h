#ifndef MYGRAPHICSSCENE_H
#define MYGRAPHICSSCENE_H

#include <QGraphicsScene>

class MyGraphicsScene : public QGraphicsScene {

public:
    explicit MyGraphicsScene(QObject* parent);
    ~MyGraphicsScene();
    void RenderPdf(QPainter* painter);

    QGraphicsPathItem* getItemZero() const;
    QGraphicsPathItem* getItemHome() const;

private:
    bool drawPdf;
    QGraphicsPathItem* itemZero;
    QGraphicsPathItem* itemHome;

    // QGraphicsScene interface
protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void drawForeground(QPainter* painter, const QRectF& rect) override;
};

#endif // MYGRAPHICSSCENE_H
