#ifndef MYGRAPHICSSCENE_H
#define MYGRAPHICSSCENE_H

#include <QGraphicsItem>
#include <QGraphicsScene>

class Scene : public QGraphicsScene {
    friend class MainWindow;

public:
    explicit Scene(QObject* parent = nullptr);
    ~Scene();
    void RenderPdf();
    QRectF itemsBoundingRect();
    static bool drawPdf();
    static QList<QGraphicsItem*> selectedItems();
    static void addItem(QGraphicsItem* item);
    static QList<QGraphicsItem*> items(Qt::SortOrder order = Qt::DescendingOrder);
    void setCross(const QPointF& cross);

private:
    bool m_drawPdf;
    QPointF m_cross;

    // QGraphicsScene interface
protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void drawForeground(QPainter* painter, const QRectF& rect) override;
};

#endif // MYGRAPHICSSCENE_H
