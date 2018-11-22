#include "point.h"
#include "mainwindow.h"

#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include "forms/materialsetupform.h"

#include <QSettings>
#include <clipper.hpp>

#include "filetree/FileHolder.h"
#include "gi/graphicsitem.h"

using namespace ClipperLib;

QRectF worckRect;

void updateRect()
{
    //    Clipper clipper;
    //    clipper.AddPaths(FileHolder::getPaths(), ptSubject, true);
    //    IntRect r(clipper.GetBounds());
    //    boardRect.setTopLeft(QPointF(r.left * dScale, r.top * dScale));
    //    boardRect.setBottomRight(QPointF(r.right * dScale, r.bottom * dScale));
    if (MyScene::self)
        worckRect = MyScene::self->itemsBoundingRect();
}

Point::Point(int type)
    : m_type(type)
{
    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsMovable);
    if (m_type == Home) {
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 0, 90);
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 270, -90);
        m_color = QColor(0, 255, 0, 120);
        setToolTip("G-Code Home Point");
    } else {
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 90, 90);
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 360, -90);
        m_color = QColor(255, 0, 0, 120);
        setToolTip("G-Code Zero Point");
    }
    m_shape.addEllipse(QRectF(QPointF(-3, -3), QSizeF(6, 6)));
    m_rect = m_path.boundingRect();
}

QRectF Point::boundingRect() const
{
    if (MyScene::self && MyScene::self->m_drawPdf)
        return QRectF();
    return m_rect;
}

void Point::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    if (MyScene::self && MyScene::self->m_drawPdf)
        return;

    QColor c(m_color);
    if (option->state & QStyle ::State_MouseOver)
        c.setAlpha(255);

    painter->setPen(Qt::NoPen);
    painter->setBrush(c);
    painter->drawPath(m_path);
    painter->setPen(c);
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPoint(0, 0), 2, 2);
}

QPainterPath Point::shape() const
{
    if (MyScene::self && MyScene::self->m_drawPdf)
        return QPainterPath();

    return m_shape;
}

void Point::resetPos()
{
    updateRect();
    if (m_type == Home)
        setPos(worckRect.bottomRight());
    else
        setPos(worckRect.topLeft());
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
    if (m_type == Home)
        MaterialSetup::self->setHomePos(pos());
    else
        MaterialSetup::self->setZeroPos(pos());
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
QVector<Shtift*> Shtift::m_shtifts;

Shtift::Shtift()
    : QGraphicsItem(nullptr)
{
    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsMovable);

    if (m_shtifts.size() % 2) {
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 0, 90);
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 270, -90);
    } else {
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 90, 90);
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 360, -90);
    }
    m_shape.addEllipse(QRectF(QPointF(-3, -3), QSizeF(6, 6)));
    m_rect = m_path.boundingRect();

    setToolTip("Штифт " + QString::number(m_shtifts.size() + 1));
    setZValue(std::numeric_limits<qreal>::max() - m_shtifts.size());

    QSettings settings;
    settings.beginGroup("Shtift" + QString::number(m_shtifts.size()));
    setPos(settings.value("pos").toPointF());

    m_shtifts.append(this);
    MyScene::self->addItem(this);
}

Shtift::~Shtift()
{
    QSettings settings;
    settings.beginGroup("Shtift" + QString::number(m_shtifts.indexOf(this)));
    settings.setValue("pos", pos());
}

QRectF Shtift::boundingRect() const
{
    if (MyScene::self != nullptr && MyScene::self->m_drawPdf)
        return QRectF();

    return m_rect;
}

void Shtift::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    if (MyScene::self && MyScene::self->m_drawPdf)
        return;

    QColor c(255, 255, 0, 120);
    if (option->state & QStyle ::State_MouseOver)
        c.setAlpha(255);

    painter->setPen(Qt::NoPen);
    painter->setBrush(c);
    painter->drawPath(m_path);
    painter->setPen(c);
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPoint(0, 0), 2, 2);
}

QPainterPath Shtift::shape() const
{
    if (MyScene::self && MyScene::self->m_drawPdf)
        return QPainterPath();
    return m_shape;
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
        if (p[0].x() > worckRect.left() + worckRect.width() * 0.5)
            p[0].rx() = worckRect.left() + worckRect.width() * 0.5;
        if (p[0].y() > worckRect.top() + worckRect.height() * 0.5)
            p[0].ry() = worckRect.top() + worckRect.height() * 0.5;
        p[2] = m_shtifts[2]->m_lastPos - (p[0] - m_lastPos);
        p[1].rx() = p[2].x();
        p[1].ry() = p[0].y();
        p[3].rx() = p[0].x();
        p[3].ry() = p[2].y();
        break;
    case 1:
        if (p[1].x() < worckRect.left() + worckRect.width() * 0.5)
            p[1].rx() = worckRect.left() + worckRect.width() * 0.5;
        if (p[1].y() > worckRect.top() + worckRect.height() * 0.5)
            p[1].ry() = worckRect.top() + worckRect.height() * 0.5;
        p[3] = m_shtifts[3]->m_lastPos - (p[1] - m_lastPos);
        p[0].rx() = p[3].x();
        p[0].ry() = p[1].y();
        p[2].rx() = p[1].x();
        p[2].ry() = p[3].y();
        break;
    case 2:
        if (p[2].x() < worckRect.left() + worckRect.width() * 0.5)
            p[2].rx() = worckRect.left() + worckRect.width() * 0.5;
        if (p[2].y() < worckRect.top() + worckRect.height() * 0.5)
            p[2].ry() = worckRect.top() + worckRect.height() * 0.5;
        p[0] = m_shtifts[0]->m_lastPos - (p[2] - m_lastPos);
        p[1].rx() = p[2].x();
        p[1].ry() = p[0].y();
        p[3].rx() = p[0].x();
        p[3].ry() = p[2].y();
        break;
    case 3:
        if (p[3].x() > worckRect.left() + worckRect.width() * 0.5)
            p[3].rx() = worckRect.left() + worckRect.width() * 0.5;
        if (p[3].y() < worckRect.top() + worckRect.height() * 0.5)
            p[3].ry() = worckRect.top() + worckRect.height() * 0.5;
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
    if (!(flags() & QGraphicsItem::ItemIsMovable))
        return;
    resetPos();
    QGraphicsItem::mouseDoubleClickEvent(event);
}

void Shtift::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    updateRect();
    for (int i = 0; i < 4; ++i)
        m_shtifts[i]->m_lastPos = m_shtifts[i]->pos();
    QGraphicsItem::mousePressEvent(event);
}

void Shtift::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) { QGraphicsItem::mouseReleaseEvent(event); }

int Point::type() const { return m_type ? PointHomeType : PointZeroType; }

int Shtift::type() const { return ShtiftType; }

QVector<Shtift*> Shtift::shtifts() { return m_shtifts; }

void Shtift::resetPos()
{
    updateRect();
    QPointF p[]{
        QPointF(worckRect.topLeft() + QPointF(-3, -3)),
        QPointF(worckRect.topRight() + QPointF(+3, -3)),
        QPointF(worckRect.bottomRight() + QPointF(+3, +3)),
        QPointF(worckRect.bottomLeft() + QPointF(-3, +3)),
    };

    for (int i = 0; i < 4; ++i)
        m_shtifts[i]->setPos(p[i]);
}
