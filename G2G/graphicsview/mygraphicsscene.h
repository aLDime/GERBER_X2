#ifndef MYGRAPHICSSCENE_H
#define MYGRAPHICSSCENE_H

#include <QGraphicsScene>

class MyGraphicsScene : public QGraphicsScene {

public:
    MyGraphicsScene(QObject* parent);
    void RenderPdf(QPainter* painter);

private:
    bool drawPdf;

    // QGraphicsScene interface
protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void drawForeground(QPainter* painter, const QRectF& rect) override;
};

#endif // MYGRAPHICSSCENE_H
