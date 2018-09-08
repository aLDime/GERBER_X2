#include "point.h"
#include "mainwindow.h"

#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include "forms/materialsetupform.h"

#include <QSettings>
#include <clipper.hpp>

#include <filetree/gerberitem.h>

using namespace ClipperLib;

QRectF boardRect;

void updateRect()
{
    Clipper clipper;
    for (G::File*& f : GerberItem::gFiles) {
        if (f->itemGroup->isVisible())
            clipper.AddPaths(f->mergedPaths, ptSubject, true);
    }
    IntRect r(clipper.GetBounds());
    boardRect.setTopLeft(QPointF(r.left * dScale, r.top * dScale));
    boardRect.setBottomRight(QPointF(r.right * dScale, r.bottom * dScale));
}

Point::Point(int type)
    : m_pen(Qt::NoPen)
    , m_type(type)
{
    setFlags(QGraphicsItem::ItemIsMovable);
    if (m_type == HOME) {
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 0, 90);
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 270, -90);
        m_brush = QBrush(QColor(0, 255, 0, 64));
        setToolTip("G-Code Home Point");
    } else {
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 90, 90);
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 360, -90);
        m_brush = QBrush(QColor(255, 0, 0, 64));
        setToolTip("G-Code Zero Point");
    }
    m_shape.addEllipse(QRectF(QPointF(-3, -3), QSizeF(6, 6)));
    m_rect = m_path.boundingRect();
}

QRectF Point::boundingRect() const
{
    if (MyScene::self != nullptr && MyScene::self->drawPdf)
        return QRectF();
    return m_rect;
}

void Point::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    if (reinterpret_cast<MyScene*>(scene())->drawPdf)
        return;

    painter->setPen(Qt::NoPen);
    painter->setBrush(m_brush);
    painter->drawPath(m_path);
    painter->setPen(QPen(m_brush.color(), 1.0));
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPoint(0, 0), 2, 2);
}

QPainterPath Point::shape() const
{
    //    if (MyScene::self != nullptr && MyScene::self->drawPdf)
    //        return QPainterPath();
    return m_shape;
}

void Point::resetPos()
{
    updateRect();
    if (m_type == HOME)
        setPos(boardRect.bottomRight());
    else
        setPos(boardRect.topLeft());
}

void Point::setPos(const QPointF& pos)
{
    QGraphicsItem::setPos(pos);
    updateMaterialSetupForm();
}

void Point::setPosX(double x)
{
    QPointF pos_(pos());
    pos_.setX(x);
    QGraphicsItem::setPos(pos_);
    updateMaterialSetupForm();
}

void Point::setPosY(double y)
{
    QPointF pos_(pos());
    pos_.setY(y);
    QGraphicsItem::setPos(pos_);
    updateMaterialSetupForm();
}

void Point::updateMaterialSetupForm()
{
    if (m_type == HOME)
        MaterialSetupForm::self->setHomePos(pos());
    else
        MaterialSetupForm::self->setZeroPos(pos());
}

void Point::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    updateMaterialSetupForm();
    QGraphicsItem::mouseMoveEvent(event);
}

void Point::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    resetPos();
    updateMaterialSetupForm();
    QGraphicsItem::mouseDoubleClickEvent(event);
}

////////////////////////////////////////////////
/// \brief Shtift::Shtift
/// \param type
/// \param num
///
Shtift::Shtift(int num)
    : m_pen(Qt::NoPen)
{
    setFlags(QGraphicsItem::ItemIsMovable);
    if (num % 2) {
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 0, 90);
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 270, -90);
    } else {
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 90, 90);
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 360, -90);
    }
    m_shape.addEllipse(QRectF(QPointF(-3, -3), QSizeF(6, 6)));
    m_rect = m_path.boundingRect();
    QSettings settings;
    settings.beginGroup("Shtift" + QString::number(num));
    setPos(settings.value("pos").toPointF());
}

Shtift::~Shtift()
{
    QSettings settings;
    settings.beginGroup("Shtift" + QString::number(m_shtifts.indexOf(this)));
    settings.setValue("pos", pos());
}

QRectF Shtift::boundingRect() const
{
    if (MyScene::self != nullptr && MyScene::self->drawPdf)
        return QRectF();
    return m_rect;
}

void Shtift::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    if (reinterpret_cast<MyScene*>(scene())->drawPdf)
        return;

    painter->setPen(Qt::NoPen);
    painter->setBrush(m_brush);
    painter->drawPath(m_path);
    painter->setPen(QPen(m_brush.color(), 1.0));
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPoint(0, 0), 2, 2);
}

QPainterPath Shtift::shape() const
{
    //    if (MyScene::self != nullptr && MyScene::self->drawPdf)
    //        return QPainterPath();
    return m_shape;
}

void Shtift::setBrush(const QBrush& brush)
{
    m_brush = brush;
}

void Shtift::setShtifts(const QVector<Shtift*>& shtifts)
{
    m_shtifts = shtifts;
}

void Shtift::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseMoveEvent(event);

    QPointF p[4]{
        m_shtifts[0]->pos(),
        m_shtifts[1]->pos(),
        m_shtifts[2]->pos(),
        m_shtifts[3]->pos()
    };

    switch (m_shtifts.indexOf(this)) {
    case 0:
        if (p[0].x() > boardRect.left() + boardRect.width() * 0.5)
            p[0].rx() = boardRect.left() + boardRect.width() * 0.5;
        if (p[0].y() > boardRect.top() + boardRect.height() * 0.5)
            p[0].ry() = boardRect.top() + boardRect.height() * 0.5;
        p[2] = m_shtifts[2]->m_lastPos - (p[0] - m_lastPos);
        p[1].rx() = p[2].x();
        p[1].ry() = p[0].y();
        p[3].rx() = p[0].x();
        p[3].ry() = p[2].y();
        break;
    case 1:
        if (p[1].x() < boardRect.left() + boardRect.width() * 0.5)
            p[1].rx() = boardRect.left() + boardRect.width() * 0.5;
        if (p[1].y() > boardRect.top() + boardRect.height() * 0.5)
            p[1].ry() = boardRect.top() + boardRect.height() * 0.5;
        p[3] = m_shtifts[3]->m_lastPos - (p[1] - m_lastPos);
        p[0].rx() = p[3].x();
        p[0].ry() = p[1].y();
        p[2].rx() = p[1].x();
        p[2].ry() = p[3].y();
        break;
    case 2:
        if (p[2].x() < boardRect.left() + boardRect.width() * 0.5)
            p[2].rx() = boardRect.left() + boardRect.width() * 0.5;
        if (p[2].y() < boardRect.top() + boardRect.height() * 0.5)
            p[2].ry() = boardRect.top() + boardRect.height() * 0.5;
        p[0] = m_shtifts[0]->m_lastPos - (p[2] - m_lastPos);
        p[1].rx() = p[2].x();
        p[1].ry() = p[0].y();
        p[3].rx() = p[0].x();
        p[3].ry() = p[2].y();
        break;
    case 3:
        if (p[3].x() > boardRect.left() + boardRect.width() * 0.5)
            p[3].rx() = boardRect.left() + boardRect.width() * 0.5;
        if (p[3].y() < boardRect.top() + boardRect.height() * 0.5)
            p[3].ry() = boardRect.top() + boardRect.height() * 0.5;
        p[1] = m_shtifts[1]->m_lastPos - (p[3] - m_lastPos);
        p[0].rx() = p[3].x();
        p[0].ry() = p[1].y();
        p[2].rx() = p[1].x();
        p[2].ry() = p[3].y();
        break;
    }
    for (int i = 0; i < 4; ++i)
        m_shtifts[i]->setPos(p[i]);
}

void Shtift::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    updateRect();
    QPointF p[]{
        QPointF(boardRect.topLeft() + QPointF(-3, -3)),
        QPointF(boardRect.topRight() + QPointF(+3, -3)),
        QPointF(boardRect.bottomRight() + QPointF(+3, +3)),
        QPointF(boardRect.bottomLeft() + QPointF(-3, +3)),
    };

    for (int i = 0; i < 4; ++i)
        m_shtifts[i]->setPos(p[i]);

    QGraphicsItem::mouseDoubleClickEvent(event);
}

void Shtift::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (boardRect.isEmpty())
        updateRect();
    for (int i = 0; i < 4; ++i)
        m_shtifts[i]->m_lastPos = m_shtifts[i]->pos();
    QGraphicsItem::mousePressEvent(event);
}

void Shtift::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseReleaseEvent(event);
}
