#ifndef MYGRAPHICSSCENE_H
#define MYGRAPHICSSCENE_H

#include <QGraphicsItem>
#include <QGraphicsScene>

class MyScene : public QGraphicsScene {
    friend class Point;
    friend class Shtift;

public:
    explicit MyScene(QObject* parent);
    ~MyScene();
    void RenderPdf();

    static MyScene* self;
    QRectF itemsBoundingRect();

    bool drawPdf() const;

private:
    bool m_drawPdf;

    // QGraphicsScene interface
protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void drawForeground(QPainter* painter, const QRectF& rect) override;
};

#endif // MYGRAPHICSSCENE_H
