#ifndef VIEW_H
#define VIEW_H

#include "mygraphicsview.h"

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QSettings>

class QDRuler;

class MyGraphicsView : public QGraphicsView {
    Q_OBJECT
public:
    explicit MyGraphicsView(QWidget* parent = 0);
    ~MyGraphicsView();
    void setScene(QGraphicsScene* Scene);
    void zoom100();
    void zoomFit();
    void zoomIn();
    void zoomOut();
    static MyGraphicsView* self;

signals:
    void fileDroped(const QString& file);

private:
    double zoom = 100;
    double numScheduledScalings = 0;
    QDRuler* hRuler;
    QDRuler* vRuler;
    QPointF centerPoint;
    const double zoomFactor = 1.5;

    //    void AnimFinished()
    //    {
    //        if (numScheduledScalings > 0) {
    //            numScheduledScalings--;
    //        } else {
    //            numScheduledScalings++;
    //        }
    //        sender()->~QObject();
    //        UpdateRuler();
    //    }
    //    void ScalingTime(qreal x)
    //    {
    //        qreal factor = 1.0 + qreal(numScheduledScalings) / 100 * x;
    //        qDebug() << numScheduledScalings << x << transform().m11();
    //        scale(factor, factor);
    //    }
    void UpdateRuler();

    // QWidget interface
protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
};

#endif // VIEW_H
