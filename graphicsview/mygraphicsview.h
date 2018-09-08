#ifndef VIEW_H
#define VIEW_H

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QSettings>

class QDRuler;

class MyGraphicsView : public QGraphicsView {
    Q_OBJECT
public:
    explicit MyGraphicsView(QWidget* parent = 0);
    ~MyGraphicsView();
    void SetScene(QGraphicsScene* Scene);
    void Zoom100();
    void ZoomFit();
    void ZoomIn();
    void ZoomOut();
    static MyGraphicsView* self;

signals:
    void FileDroped(const QString& file);
    void ClearScene();

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
    virtual void contextMenuEvent(QContextMenuEvent* event) override;
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;

    //    // QWidget interface
    //protected:
    //    virtual void mousePressEvent(QMouseEvent* event) override
    //    {
    //        //    qDebug() << "mousePressEvent";
    //        //    if (event->buttons() & Qt::MiddleButton) {
    //        //        setInteractive(false);
    //        //        // по нажатию средней кнопки мыши создаем событие ее отпускания выставляем моду перетаскивания и создаем событие зажатой левой кнопки мыши
    //        //        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, event->localPos(), event->screenPos(), event->windowPos(), Qt::LeftButton, 0, event->modifiers());
    //        //        QGraphicsView::mouseReleaseEvent(&releaseEvent);
    //        //        setDragMode(ScrollHandDrag);
    //        //        QMouseEvent fakeEvent(event->type(), event->localPos(), event->screenPos(), event->windowPos(), Qt::LeftButton, event->buttons() | Qt::LeftButton, event->modifiers());
    //        //        QGraphicsView::mousePressEvent(&fakeEvent);
    //        //    }
    //        //    else if (event->button() == Qt::RightButton) {
    //        //        // это что бы при вызове контекстного меню ничего постороннего не было
    //        //        setDragMode(NoDrag);
    //        //        QGraphicsView::mousePressEvent(event);
    //        //    }
    //        //    else {
    //        //        // это для выделения рамкой  - работа по-умолчанию левой кнопки мыши
    //        //        QGraphicsView::mousePressEvent(event);
    //        //    }
    //        QGraphicsView::mousePressEvent(event);
    //    }
    //    virtual void mouseReleaseEvent(QMouseEvent* event) override
    //    {
    //        //    qDebug() << "mouseReleaseEvent";
    //        //    if (event->button() == Qt::MiddleButton) {
    //        //        // отпускаем левую кнопку мыши которую виртуально зажали в mousePressEvent
    //        //        QMouseEvent fakeEvent(event->type(), event->localPos(), event->screenPos(), event->windowPos(), Qt::LeftButton, event->buttons() & ~Qt::LeftButton, event->modifiers());
    //        //        QGraphicsView::mouseReleaseEvent(&fakeEvent);
    //        //        setDragMode(RubberBandDrag);
    //        //        setInteractive(true);
    //        //    }
    //        //    else {
    //        //        QGraphicsView::mouseReleaseEvent(event);
    //        //    }
    //        QGraphicsView::mouseReleaseEvent(event);
    //    }
    //    virtual void mouseMoveEvent(QMouseEvent* event) override
    //    {
    //        //qDebug() << "mouseMoveEvent";
    //        QGraphicsView::mouseMoveEvent(event);
    //    }
};

#endif // VIEW_H
