#ifndef MYGRAPHICSSCENE_H
#define MYGRAPHICSSCENE_H

#include <QGraphicsItem>
#include <QGraphicsScene>

class Scene : public QGraphicsScene {
    friend class MainWindow;

public:
    explicit Scene(QObject* parent);
    ~Scene();
    void RenderPdf();

    static Scene* self;
    QRectF itemsBoundingRect();

    static bool drawPdf();
    static QList<QGraphicsItem*> selectedItems();
    static void addItem(QGraphicsItem* item)
    {
        if (self)
            self->QGraphicsScene::addItem(item);
    }

private:
    bool m_drawPdf;

    // QGraphicsScene interface
protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void drawForeground(QPainter* painter, const QRectF& rect) override;
};

#endif // MYGRAPHICSSCENE_H
