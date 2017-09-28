#include "mygraphicsscene.h"
#include "mygraphicsview.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QPainter>
#include <QtMath>
#include <QGraphicsView>
#ifdef G2G
#include "gerberfileholder.h"
#endif
MyGraphicsScene::MyGraphicsScene(QObject* parent)
    : QGraphicsScene(parent)
    , drawPdf(false)
    , scale(0.0)
{
}

void MyGraphicsScene::RenderPdf(QPainter* painter)
{
    drawPdf = true;
    render(painter);
    drawPdf = false;
}

bool MyGraphicsScene::GetDrawPdf() const
{
    return drawPdf;
}

void MyGraphicsScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    if (drawPdf)
        return;
    painter->fillRect(rect, Qt::black);
    if (itemsBoundingRect().width() == 0 || itemsBoundingRect().height() == 0)
        return;

    painter->save();
#ifdef G2G
    if (GerberFileHolder::getDrawingType() == GerberFileHolder::RAW)
        painter->setBrush(QColor(128, 128, 0));
    else
        painter->setBrush(Qt::NoBrush);
#else
    painter->setBrush(Qt::NoBrush);
#endif

    painter->setPen(QPen(Qt::white, 0.0));
    //    painter->setPen(Qt::NoPen);
    //    painter->setBrush(Qt::NoBrush);
    painter->drawRect(itemsBoundingRect() /*+ QMarginsF(2, 2, 2, 2)*/);
    painter->restore();
}

void MyGraphicsScene::drawForeground(QPainter* painter, const QRectF& rect)
{
    if (drawPdf)
        return;
    scale = views().at(0)->matrix().m11();
    if (qFuzzyIsNull(scale))
        return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);

    QMap<long, int> hGrid;
    QMap<long, int> vGrid;
    const int a = 100;
    QVector<QColor> color = {
        QColor(a, a, a, 100),
        QColor(a, a, a, 150),
        QColor(a, a, a, 200)
    };

    //decimal = 0.1;
    double gridStep = qPow(10.0, qCeil(log10((7.0 / scale))));
    const long k = 10000;
    const double invK = 1.0 / k;
    for (long hPos = qFloor(rect.left() / gridStep) * gridStep * k, right = rect.right() * k, step = gridStep * k; hPos < right; hGrid[hPos] = 0, hPos += step) {
    }
    for (long vPos = qFloor(rect.top() / gridStep) * gridStep * k, bottom = rect.bottom() * k, step = gridStep * k; vPos < bottom; vGrid[vPos] = 0, vPos += step) {
    }
    //half = 0.5;
    gridStep *= 5;
    for (long hPos = qFloor(rect.left() / gridStep) * gridStep * k, right = rect.right() * k, step = gridStep * k; hPos < right; hGrid[hPos] = 1, hPos += step) {
    }
    for (long vPos = qFloor(rect.top() / gridStep) * gridStep * k, bottom = rect.bottom() * k, step = gridStep * k; vPos < bottom; vGrid[vPos] = 1, vPos += step) {
    }
    //integer = 1.0;
    gridStep *= 2;
    for (long hPos = qFloor(rect.left() / gridStep) * gridStep * k, right = rect.right() * k, step = gridStep * k; hPos < right; hGrid[hPos] = 2, hPos += step) {
    }
    for (long vPos = qFloor(rect.top() / gridStep) * gridStep * k, bottom = rect.bottom() * k, step = gridStep * k; vPos < bottom; vGrid[vPos] = 2, vPos += step) {
    }

    for (int i = 0; i < 3; ++i) {
        painter->setPen(QPen(color[i], 0.0 /*1.0 / scale*/));
        for (long hPos : hGrid.keys(i)) {
            if (hPos != 0)
                painter->drawLine(QLineF(hPos * invK, rect.top(), hPos * invK, rect.bottom()));
        }
        for (long vPos : vGrid.keys(i)) {
            if (vPos != 0)
                painter->drawLine(QLineF(rect.left(), vPos * invK, rect.right(), vPos * invK));
        }
    }

    painter->setPen(QPen(QColor(255, 0, 0, 255), 0.0 /*1.0 / scale*/));
    painter->drawLine(QLineF(0.5 / scale, rect.top(), 0.5 / scale, rect.bottom()));
    painter->drawLine(QLineF(rect.left(), -0.5 / scale, rect.right(), -0.5 / scale));
    painter->restore();
}
