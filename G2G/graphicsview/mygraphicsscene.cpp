#include "mygraphicsscene.h"
#include "mygraphicsview.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QPainter>
#include <QtMath>
#include <QGraphicsView>
#include "gerberfileholder.h"

MyGraphicsScene::MyGraphicsScene(QObject* parent)
    : QGraphicsScene(parent)
    , drawPdf(false)
{
    QPainterPath path;
    path.addEllipse(QPointF(), 3.0, 3.0);
    double l = 4;
    path.moveTo(QPointF(0, -l));
    path.lineTo(QPointF(0, +l));

    path.moveTo(QPointF(-l, 0));
    path.lineTo(QPointF(+l, 0));

    itemZero = new QGraphicsPathItem(path);
    itemZero->setBrush(Qt::NoBrush);
    itemZero->setPen(QPen(Qt::green, 1.0, Qt::SolidLine, Qt::SquareCap));
    itemZero->setToolTip("G-Code Zero Point");
    itemZero->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    addItem(itemZero);

    itemHome = new QGraphicsPathItem(path);
    itemHome->setBrush(Qt::NoBrush);
    itemHome->setPen(QPen(Qt::blue, 1.0, Qt::SolidLine, Qt::SquareCap));
    itemHome->setToolTip("G-Code Home Point");
    itemHome->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    addItem(itemHome);

    QSettings settings;
    settings.beginGroup("ToolpathNameForm");
    itemHome->setPos(settings.value("HomeXY").toPointF());
    itemZero->setPos(settings.value("ZeroXY").toPointF());
    settings.endGroup();
}

MyGraphicsScene::~MyGraphicsScene()
{
    QSettings settings;
    settings.beginGroup("ToolpathNameForm");
    settings.setValue("HomeXY", itemHome->pos());
    settings.setValue("ZeroXY", itemZero->pos());
    settings.endGroup();
}

void MyGraphicsScene::RenderPdf(QPainter* painter)
{
    drawPdf = true;
    render(painter);
    drawPdf = false;
}

QGraphicsPathItem* MyGraphicsScene::getItemZero() const
{
    return itemZero;
}

QGraphicsPathItem* MyGraphicsScene::getItemHome() const
{
    return itemHome;
}

void MyGraphicsScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    if (drawPdf)
        return;
    painter->fillRect(rect, Qt::black);
    if (itemsBoundingRect().width() == 0 || itemsBoundingRect().height() == 0)
        return;

    painter->save();

    //    if (GerberFileHolder::getDrawingType() == GerberFileHolder::RAW)
    //        painter->setBrush(QColor(128, 128, 0));
    //    else
    painter->setBrush(Qt::NoBrush);

    painter->setPen(QPen(Qt::white, 0.0));
    painter->drawRect(itemsBoundingRect() /*+ QMarginsF(2, 2, 2, 2)*/);
    painter->restore();
}

void MyGraphicsScene::drawForeground(QPainter* painter, const QRectF& rect)
{
    if (drawPdf)
        return;
    double scale = views().at(0)->matrix().m11();
    if (qFuzzyIsNull(scale))
        return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);

    QMap<long, long> hGrid;
    QMap<long, long> vGrid;

    const int с = 100;
    QVector<QColor> color = { QColor(с, с, с, 85), QColor(с, с, с, 170), QColor(с, с, с) };

    const long k = 10000;
    const double invK = 1.0 / k;

    double gridStep = qPow(10.0, qCeil(log10((7.0 / scale)))); //decimal = 0.1;
    for (long hPos = qFloor(rect.left() / gridStep) * gridStep * k, right = rect.right() * k, step = gridStep * k; hPos < right; hPos += step)
        hGrid[hPos] = 0;
    for (long vPos = qFloor(rect.top() / gridStep) * gridStep * k, bottom = rect.bottom() * k, step = gridStep * k; vPos < bottom; vPos += step)
        vGrid[vPos] = 0;

    gridStep *= 5; //half = 0.5;
    for (long hPos = qFloor(rect.left() / gridStep) * gridStep * k, right = rect.right() * k, step = gridStep * k; hPos < right; hPos += step)
        hGrid[hPos] = 1;
    for (long vPos = qFloor(rect.top() / gridStep) * gridStep * k, bottom = rect.bottom() * k, step = gridStep * k; vPos < bottom; vPos += step)
        vGrid[vPos] = 1;

    gridStep *= 2; //integer = 1.0;
    for (long hPos = qFloor(rect.left() / gridStep) * gridStep * k, right = rect.right() * k, step = gridStep * k; hPos < right; hPos += step)
        hGrid[hPos] = 2;
    for (long vPos = qFloor(rect.top() / gridStep) * gridStep * k, bottom = rect.bottom() * k, step = gridStep * k; vPos < bottom; vPos += step)
        vGrid[vPos] = 2;

    for (int i = 0; i < 3; ++i) {
        painter->setPen(QPen(color[i], 1.0 / scale));
        for (long hPos : hGrid.keys(i)) {
            if (hPos)
                painter->drawLine(QLineF(hPos * invK, rect.top(), hPos * invK, rect.bottom()));
        }
        for (long vPos : vGrid.keys(i)) {
            if (vPos)
                painter->drawLine(QLineF(rect.left(), vPos * invK, rect.right(), vPos * invK));
        }
    }

    painter->setPen(QPen(QColor(255, 0, 0, 255), 0.0 /*1.0 / scale*/));
    painter->drawLine(QLineF(0.5 / scale, rect.top(), 0.5 / scale, rect.bottom()));
    painter->drawLine(QLineF(rect.left(), -0.5 / scale, rect.right(), -0.5 / scale));
    painter->restore();
}
