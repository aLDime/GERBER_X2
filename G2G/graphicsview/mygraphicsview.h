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
    void SetScene(QGraphicsScene* Scene);
    void Setup(QSettings& settings);
    void Zoom100();
    void ZoomFit();
    void ZoomIn();
    void ZoomOut();

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

    void AnimFinished();
    void ScalingTime(qreal x);
    void UpdateRuler();

    // QWidget interface
protected:
    virtual void contextMenuEvent(QContextMenuEvent* event) override;
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;

    // QWidget interface
protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
};

#endif // VIEW_H
