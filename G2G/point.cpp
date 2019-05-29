#include "point.h"
#include "filetree/fileholder.h"
#include "forms/gcodepropertiesform.h"
#include "gi/graphicsitem.h"
#include "mainwindow.h"
#include "settings.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QSettings>
#include <clipper.hpp>

using namespace ClipperLib;

QVector<Shtift*> Shtift::m_shtifts;
QRectF Shtift::worckRect;

bool updateRect()
{
    QRectF rect(FileHolder::getSelectedBoundingRect());
    if (rect.isEmpty()) {
        if (QMessageBox::question(nullptr, "", QObject::tr("There is no dedicated data to define boundaries.\nOld data will be used."), QMessageBox::No, QMessageBox::Yes)
            == QMessageBox::No)
            return false;
        return true;
    }
    Shtift::worckRect = rect;
    return true;
}

Point::Point(int type)
    : m_type(type)
{
    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsMovable);
    if (m_type == Home) {
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 0, 90);
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 270, -90);
        setToolTip(QObject::tr("G-Code Home Point"));
    } else {
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 90, 90);
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 360, -90);
        setToolTip(QObject::tr("G-Code Zero Point"));
    }
    m_shape.addEllipse(QRectF(QPointF(-3, -3), QSizeF(6, 6)));
    m_rect = m_path.boundingRect();

    QSettings settings;
    settings.beginGroup("Point");
    setPos(settings.value("pos" + QString::number(m_type)).toPointF());
    setFlag(QGraphicsItem::ItemIsMovable, settings.value("fixed").toBool());
}

Point::~Point()
{
    QSettings settings;
    settings.beginGroup("Point");
    settings.setValue("pos" + QString::number(m_type), pos());
    settings.setValue("fixed", bool(flags() & QGraphicsItem::ItemIsMovable));
}

QRectF Point::boundingRect() const
{
    if (Scene::drawPdf())
        return QRectF();
    return m_rect;
}

void Point::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    if (Scene::drawPdf())
        return;

    QColor c(m_type == Home ? Settings::color(Colors::Home) : Settings::color(Colors::Zero));
    if (option->state & QStyle ::State_MouseOver)
        c.setAlpha(255);
    if (!(flags() & QGraphicsItem::ItemIsMovable))
        c.setAlpha(50);

    painter->setPen(Qt::NoPen);
    painter->setBrush(c);
    painter->drawPath(m_path);
    painter->setPen(c);
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPoint(0, 0), 2, 2);
}

QPainterPath Point::shape() const
{
    if (Scene::drawPdf())
        return QPainterPath();

    return m_shape;
}

void Point::resetPos()
{
    if (updateRect())
        if (m_type == Home)
            setPos(Shtift::worckRect.bottomRight());
        else
            setPos(Shtift::worckRect.topLeft());
    updateGCPForm();
}

void Point::setPosX(double x)
{
    QPointF point(pos());
    if (point.x() == x)
        return;
    point.setX(x);
    setPos(point);
}

void Point::setPosY(double y)
{
    QPointF point(pos());
    if (point.y() == y)
        return;
    point.setY(y);
    setPos(point);
}

void Point::updateGCPForm()
{
    GCodePropertiesForm::updatePosDsbxs();
    QSettings settings;
    settings.beginGroup("Points");
    settings.setValue("pos" + QString::number(m_type), pos());
}

void Point::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseMoveEvent(event);
    updateGCPForm();
}

void Point::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    if (!(flags() & QGraphicsItem::ItemIsMovable))
        return;
    resetPos();
    //    QMatrix matrix(scene()->views().first()->matrix());
    //    matrix.translate(-pos().x(), pos().y());
    //    scene()->views().first()->setMatrix(matrix);
    updateGCPForm();
    QGraphicsItem::mouseDoubleClickEvent(event);
}

////////////////////////////////////////////////
/// \brief Shtift::Shtift
/// \param type
/// \param num
///

Shtift::Shtift()
    : QGraphicsItem(nullptr)
{
    setAcceptHoverEvents(true);

    if (m_shtifts.size() % 2) {
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 0, 90);
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 270, -90);
    } else {
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 90, 90);
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 360, -90);
    }
    m_shape.addEllipse(QRectF(QPointF(-3, -3), QSizeF(6, 6)));
    m_rect = m_path.boundingRect();

    setToolTip(QObject::tr("Shtift ") + QString::number(m_shtifts.size() + 1));
    setZValue(std::numeric_limits<qreal>::max() - m_shtifts.size());

    QSettings settings;
    settings.beginGroup("Shtift");
    setFlag(QGraphicsItem::ItemIsMovable, settings.value("fixed").toBool());
    setPos(settings.value("pos" + QString::number(m_shtifts.size())).toPointF());
    if (m_shtifts.isEmpty())
        worckRect = settings.value("worckRect").toRectF();
    m_shtifts.append(this);
    Scene::self->addItem(this);
}

Shtift::~Shtift()
{
    QSettings settings;
    settings.beginGroup("Shtift");
    settings.setValue("pos" + QString::number(m_shtifts.indexOf(this)), pos());
    if (!m_shtifts.indexOf(this)) {
        //Settings().writeSettings();
        settings.setValue("fixed", bool(flags() & QGraphicsItem::ItemIsMovable));
        settings.setValue("worckRect", worckRect);
    }
}

QRectF Shtift::boundingRect() const
{
    if (Scene::drawPdf())
        return QRectF();
    return m_rect;
}

void Shtift::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    if (Scene::drawPdf())
        return;

    QColor c(Settings::color(Colors::Shtift));
    if (option->state & QStyle ::State_MouseOver)
        c.setAlpha(255);
    if (!(flags() & QGraphicsItem::ItemIsMovable))
        c.setAlpha(50);

    painter->setPen(Qt::NoPen);
    painter->setBrush(c);
    painter->drawPath(m_path);
    painter->setPen(c);
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPoint(0, 0), 2, 2);
}

QPainterPath Shtift::shape() const
{
    if (Scene::drawPdf())
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
        if (p[0].x() > Shtift::worckRect.left() + Shtift::worckRect.width() * 0.5)
            p[0].rx() = Shtift::worckRect.left() + Shtift::worckRect.width() * 0.5;
        if (p[0].y() > Shtift::worckRect.top() + Shtift::worckRect.height() * 0.5)
            p[0].ry() = Shtift::worckRect.top() + Shtift::worckRect.height() * 0.5;
        p[2] = m_shtifts[2]->m_lastPos - (p[0] - m_lastPos);
        p[1].rx() = p[2].x();
        p[1].ry() = p[0].y();
        p[3].rx() = p[0].x();
        p[3].ry() = p[2].y();
        break;
    case 1:
        if (p[1].x() < Shtift::worckRect.left() + Shtift::worckRect.width() * 0.5)
            p[1].rx() = Shtift::worckRect.left() + Shtift::worckRect.width() * 0.5;
        if (p[1].y() > Shtift::worckRect.top() + Shtift::worckRect.height() * 0.5)
            p[1].ry() = Shtift::worckRect.top() + Shtift::worckRect.height() * 0.5;
        p[3] = m_shtifts[3]->m_lastPos - (p[1] - m_lastPos);
        p[0].rx() = p[3].x();
        p[0].ry() = p[1].y();
        p[2].rx() = p[1].x();
        p[2].ry() = p[3].y();
        break;
    case 2:
        if (p[2].x() < Shtift::worckRect.left() + Shtift::worckRect.width() * 0.5)
            p[2].rx() = Shtift::worckRect.left() + Shtift::worckRect.width() * 0.5;
        if (p[2].y() < Shtift::worckRect.top() + Shtift::worckRect.height() * 0.5)
            p[2].ry() = Shtift::worckRect.top() + Shtift::worckRect.height() * 0.5;
        p[0] = m_shtifts[0]->m_lastPos - (p[2] - m_lastPos);
        p[1].rx() = p[2].x();
        p[1].ry() = p[0].y();
        p[3].rx() = p[0].x();
        p[3].ry() = p[2].y();
        break;
    case 3:
        if (p[3].x() > Shtift::worckRect.left() + Shtift::worckRect.width() * 0.5)
            p[3].rx() = Shtift::worckRect.left() + Shtift::worckRect.width() * 0.5;
        if (p[3].y() < Shtift::worckRect.top() + Shtift::worckRect.height() * 0.5)
            p[3].ry() = Shtift::worckRect.top() + Shtift::worckRect.height() * 0.5;
        p[1] = m_shtifts[1]->m_lastPos - (p[3] - m_lastPos);
        p[0].rx() = p[3].x();
        p[0].ry() = p[1].y();
        p[2].rx() = p[1].x();
        p[2].ry() = p[3].y();
        break;
    }
    for (int i = 0; i < 4; ++i)
        m_shtifts[i]->setPos(p[i]);
    QSettings settings;
    settings.beginGroup("Shtift");
    settings.setValue("pos" + QString::number(m_shtifts.indexOf(this)), pos());
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
    const double k = 3.0;
    QPointF p[]{
        QPointF(Shtift::worckRect.topLeft() + QPointF(-k, -k)),
        QPointF(Shtift::worckRect.topRight() + QPointF(+k, -k)),
        QPointF(Shtift::worckRect.bottomRight() + QPointF(+k, +k)),
        QPointF(Shtift::worckRect.bottomLeft() + QPointF(-k, +k)),
    };

    for (int i = 0; i < 4; ++i)
        m_shtifts[i]->setPos(p[i]);
    QSettings settings;
    settings.beginGroup("Shtift");
    settings.setValue("pos" + QString::number(m_shtifts.indexOf(this)), pos());
}
