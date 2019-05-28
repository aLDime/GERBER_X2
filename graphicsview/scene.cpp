#include "scene.h"
#include "graphicsview.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QMenu>
#include <QPainter>
#include <QPdfWriter>
#include <QtMath>
#include <settings.h>

Scene* Scene::self = nullptr;

Scene::Scene(QObject* parent)
    : QGraphicsScene(parent)
    , m_drawPdf(false)
{
    self = this;
}

Scene::~Scene()
{
    self = nullptr;
}

void Scene::RenderPdf()
{
    QString curFile = QFileDialog::getSaveFileName(nullptr, tr("Save PDF file"), "File", tr("File(*.pdf)"));
    if (curFile.isEmpty())
        return;

    m_drawPdf = true;

    QRectF rect;

    for (QGraphicsItem* item : items())
        if (item->isVisible() && !item->boundingRect().isNull())
            rect |= item->boundingRect();

    //QRectF rect(QGraphicsScene::itemsBoundingRect());
    QSizeF size(rect.size());

    qDebug() << size << rect;

    QPdfWriter pdfWriter(curFile);
    pdfWriter.setPageSizeMM(size);
    pdfWriter.setMargins({ 0, 0, 0, 0 });
    pdfWriter.setResolution(1000000);

    QPainter painter(&pdfWriter);
    painter.setTransform(QTransform().scale(1.0, -1.0));
    painter.translate(0, -(pdfWriter.resolution() / 25.4) * size.height());
    render(&painter,
        QRectF(0, 0, pdfWriter.width(), pdfWriter.height()),
        rect, Qt::IgnoreAspectRatio);

    m_drawPdf = false;
}

QRectF Scene::itemsBoundingRect()
{
    m_drawPdf = true;
    QRectF rect(QGraphicsScene::itemsBoundingRect());
    m_drawPdf = false;
    return rect;
}

bool Scene::drawPdf()
{
    if (self)
        return self->m_drawPdf;
    return false;
}

void Scene::drawBackground(QPainter* painter, const QRectF& rect)
{
    if (m_drawPdf)
        return;

    painter->fillRect(rect, Settings::color(Colors::Background));

    //    if (qFuzzyIsNull(itemsBoundingRect().width()) || qFuzzyIsNull(itemsBoundingRect().height()))
    //        return;

    //    painter->save();

    //    //    if (GerberFileHolder::getDrawingType() == GerberFileHolder::RAW)
    //    //        painter->setBrush(QColor(128, 128, 0));
    //    //    else
    //    painter->setBrush(Qt::NoBrush);

    //    painter->setPen(QPen(Qt::white, 0.0));
    //    painter->drawRect(itemsBoundingRect() /*+ QMarginsF(2, 2, 2, 2)*/);
    //    painter->restore();
}

void Scene::drawForeground(QPainter* painter, const QRectF& rect)
{
    if (m_drawPdf)
        return;

    double scale = views().first()->matrix().m11();
    if (qFuzzyIsNull(scale))
        return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    //painter->setCompositionMode(QPainter::CompositionMode_Plus);

    QMap<long, long> hGrid;
    QMap<long, long> vGrid;

    const QColor color[3]{
        Settings::color(Colors::Grid1),
        Settings::color(Colors::Grid5),
        Settings::color(Colors::Grid10),
    };

    const long k = 10000;
    const double invK = 1.0 / k;

    double gridStep = qPow(10.0, qCeil(log10((8.0 / scale)))); // 0.1;
    for (long hPos = qFloor(rect.left() / gridStep) * gridStep * k, right = rect.right() * k, step = gridStep * k;
         hPos < right;
         hPos += step) {
        hGrid[hPos] = 0;
    }
    for (long vPos = qFloor(rect.top() / gridStep) * gridStep * k, bottom = rect.bottom() * k, step = gridStep * k;
         vPos < bottom;
         vPos += step) {
        vGrid[vPos] = 0;
    }

    gridStep *= 5; // 0.5;
    for (long hPos = qFloor(rect.left() / gridStep) * gridStep * k, right = rect.right() * k, step = gridStep * k;
         hPos < right;
         hPos += step) {
        hGrid[hPos] = 1;
    }
    for (long vPos = qFloor(rect.top() / gridStep) * gridStep * k, bottom = rect.bottom() * k, step = gridStep * k;
         vPos < bottom;
         vPos += step) {
        vGrid[vPos] = 1;
    }

    gridStep *= 2; // 1.0;
    for (long hPos = qFloor(rect.left() / gridStep) * gridStep * k, right = rect.right() * k, step = gridStep * k;
         hPos < right;
         hPos += step) {
        hGrid[hPos] = 2;
    }
    for (long vPos = qFloor(rect.top() / gridStep) * gridStep * k, bottom = rect.bottom() * k, step = gridStep * k;
         vPos < bottom;
         vPos += step) {
        vGrid[vPos] = 2;
    }

    for (int i = 0; i < 3; ++i) {
        painter->setPen(QPen(color[i], 1.0 / scale));
        for (long hPos : hGrid.keys(i)) {
            if (hPos) {
                painter->drawLine(QLineF(hPos * invK, rect.top(), hPos * invK, rect.bottom()));
            }
        }
        for (long vPos : vGrid.keys(i)) {
            if (vPos) {
                painter->drawLine(QLineF(rect.left(), vPos * invK, rect.right(), vPos * invK));
            }
        }
    }

    painter->setPen(QPen(QColor(255, 0, 0, 100), 0.0 /*1.0 / scale*/));
    painter->drawLine(QLineF(0.5 / scale, rect.top(), 0.5 / scale, rect.bottom()));
    painter->drawLine(QLineF(rect.left(), -0.5 / scale, rect.right(), -0.5 / scale));
    painter->restore();
}
