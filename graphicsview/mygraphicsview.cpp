#include "mygraphicsview.h"
#include "edid.h"
#include "mygraphicsview.h"
#include "myscene.h"
#include "qdruler.h"
#include <QDebug>
#include <QGLWidget>
#include <QSettings>
#include <QTimer>
#include <QTransform>
#include <QtWidgets>

MyGraphicsView* MyGraphicsView::self = nullptr;

MyGraphicsView::MyGraphicsView(QWidget* parent)
    : QGraphicsView(parent)
{

    setRenderHint(QPainter::Antialiasing, true);
    //    setOptimizationFlags(DontSavePainterState);
    //    setViewportUpdateMode(SmartViewportUpdate);
    //    setViewportUpdateMode(BoundingRectViewportUpdate);
    setDragMode(RubberBandDrag);
    //    setTransformationAnchor(AnchorUnderMouse);
    //    setResizeAnchor(AnchorUnderMouse);
    //    setAcceptDrops(true);
    setInteractive(true);
    //    setMouseTracking(true);
    ////////////////////////////////////

    // add two rulers on top and left.
    setViewportMargins(RULER_BREADTH, RULER_BREADTH, 0, 0);

    // add grid layout
    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);

    // create rulers
    hRuler = new QDRuler(QDRuler::Horizontal, this);
    vRuler = new QDRuler(QDRuler::Vertical, this);
    // add items to grid layout
    QLabel* corner = new QLabel("mm", this);
    corner->setAlignment(Qt::AlignCenter);
    corner->setFixedSize(RULER_BREADTH, RULER_BREADTH);
    gridLayout->addWidget(corner, 0, 0);
    gridLayout->addWidget(hRuler, 0, 1);
    gridLayout->addWidget(vRuler, 1, 0);
    gridLayout->addWidget(viewport(), 1, 1);
    // finally set layout
    setLayout(gridLayout);

    connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, &MyGraphicsView::UpdateRuler);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &MyGraphicsView::UpdateRuler);

    scale(1.0, -1.0); //flip vertical
    Zoom100();

    QSettings settings;
    settings.beginGroup("Viewer");
    setRenderHint(QPainter::Antialiasing, settings.value("Antialiasing", true).toBool());
    setViewport(settings.value("OpenGl").toBool() ? new QGLWidget(QGLFormat(QGL::SampleBuffers)) : new QWidget);
    viewport()->setObjectName("viewport");
    settings.endGroup();

    SetScene(new MyScene(this));
    self = this;
}

MyGraphicsView::~MyGraphicsView()
{
    self = nullptr;
}

void MyGraphicsView::SetScene(QGraphicsScene* Scene)
{
    setScene(Scene);
    UpdateRuler();
}

void MyGraphicsView::ZoomFit()
{
    scene()->setSceneRect(scene()->itemsBoundingRect());
    fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
    //scale(0.95, 0.95);
    UpdateRuler();
}

void MyGraphicsView::Zoom100()
{
    double x = 1.0, y = 1.0;
    if (0) {
        x = qAbs(1.0 / transform().m11() / (25.4 / physicalDpiX()));
        y = qAbs(1.0 / transform().m22() / (25.4 / physicalDpiY()));
    } else {
        QSizeF size(GetEdid()); // size in mm
        QRect scrGeometry = QGuiApplication::primaryScreen()->geometry(); // size in pix
        x = qAbs(1.0 / transform().m11() / (size.height() / scrGeometry.height()));
        y = qAbs(1.0 / transform().m22() / (size.width() / scrGeometry.width()));
    }
    scale(x, y);
    UpdateRuler();
}

enum {
    DURATION = 300,
    INTERVAL = 30
};

void MyGraphicsView::ZoomIn()
{
    if (transform().m11() > 10000.0)
        return;

    //    numScheduledScalings += 1;
    //    QTimeLine* anim = new QTimeLine(DURATION, this);
    //    anim->setUpdateInterval(INTERVAL);
    //    connect(anim, &QTimeLine::valueChanged, this, &MyGraphicsView::ScalingTime);
    //    connect(anim, &QTimeLine::finished, this, &MyGraphicsView::AnimFinished);
    //    anim->start();
    scale(zoomFactor, zoomFactor);
    UpdateRuler();
}

void MyGraphicsView::ZoomOut()
{
    if (transform().m11() < 1.0)
        return;

    //    numScheduledScalings -= 1;
    //    QTimeLine* anim = new QTimeLine(DURATION, this);
    //    anim->setUpdateInterval(INTERVAL);
    //    connect(anim, &QTimeLine::valueChanged, this, &MyGraphicsView::ScalingTime);
    //    connect(anim, &QTimeLine::finished, this, &MyGraphicsView::AnimFinished);
    //    anim->start();
    scale(1.0 / zoomFactor, 1.0 / zoomFactor);
    UpdateRuler();
}

void MyGraphicsView::wheelEvent(QWheelEvent* event)
{
    switch (event->modifiers()) {
    case Qt::ControlModifier:
        if (event->delta() > 0) {
            ZoomIn();
        } else {
            ZoomOut();
        }
        break;
    case Qt::ShiftModifier:
        QAbstractScrollArea::horizontalScrollBar()->setValue(QAbstractScrollArea::horizontalScrollBar()->value() - (event->delta()));
        break;
    case Qt::NoModifier:
        if (event->angleDelta().x() != 0) {
            QAbstractScrollArea::horizontalScrollBar()->setValue(QAbstractScrollArea::horizontalScrollBar()->value() - (event->delta()));
        } else {
            QAbstractScrollArea::verticalScrollBar()->setValue(QAbstractScrollArea::verticalScrollBar()->value() - (event->delta()));
        }
        break;
    default:
        QGraphicsView::wheelEvent(event);
    }
}

//void MyGraphicsView::TogglePointerMode()
//{
//    static bool selectModeButton = true;
//    setDragMode(selectModeButton ? QGraphicsView::RubberBandDrag : QGraphicsView::ScrollHandDrag);
//    //setInteractive(selectModeButton);
//    selectModeButton = !selectModeButton;
//}

void MyGraphicsView::UpdateRuler()
{
    updateSceneRect(QRectF()); //actualize mapFromScene
    QPoint p = mapFromScene(QPointF());
    vRuler->SetOrigin(p.y());
    hRuler->SetOrigin(p.x());
    vRuler->SetRulerZoom(qAbs(transform().m22() * 0.1));
    hRuler->SetRulerZoom(qAbs(transform().m11() * 0.1));
}

void MyGraphicsView::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        return;
    }
    event->ignore();
}

void MyGraphicsView::dropEvent(QDropEvent* event)
{
    if (event->mouseButtons() == Qt::LeftButton) {
        emit ClearScene();
    }
    for (QUrl& var : event->mimeData()->urls()) {
        emit FileDroped(var.toString().remove("file:///"));
    }
    event->acceptProposedAction();
}

void MyGraphicsView::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}

void MyGraphicsView::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    menu.addAction(QIcon(), tr("&Pointer Mode..."), []() { qDebug() << "&Pointer Mode..."; });
    menu.exec(event->globalPos());
    QGraphicsView::contextMenuEvent(event);
}

void MyGraphicsView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    UpdateRuler();
}
