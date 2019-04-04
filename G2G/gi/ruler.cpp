#include "ruler.h"
#include "graphicsitem.h"

#include <QDebug>
#include <QPainter>
#include <graphicsview.h>

Ruler* Ruler::self = nullptr;

Ruler::Ruler(const QPointF& point)
    : m_pt1(point)
    , m_pt2(point)
{
    //setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    self = this;
}

Ruler::~Ruler()
{
    self = nullptr;
}

QRectF Ruler::boundingRect() const
{
    const double k = 1.0 / GraphicsView ::self->matrix().m11();
    const double width = (m_textRect.width() + 10) * k;
    const double height = (m_textRect.height() + 10) * k;

    return QRectF(
               QPointF(qMin(m_pt1.x(), m_pt2.x()), qMin(m_pt1.y(), m_pt2.y())),
               QPointF(qMax(m_pt1.x(), m_pt2.x()), qMax(m_pt1.y(), m_pt2.y())))
        + QMarginsF(width, height * 2, width * 2, height);
}

void Ruler::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    QLineF line(m_pt2, m_pt1);

    if (qFuzzyIsNull(line.length()))
        return;

    const double angle = line.angle();
    const double k = 1.0 / GraphicsView ::self->matrix().m11();

    painter->setBrush(QColor(255, 255, 255, 50));
    painter->setPen(QPen(Qt::green, 1.5 * k));
    painter->drawRect(QRectF(
        QPointF(qMin(m_pt1.x(), m_pt2.x()), qMin(m_pt1.y(), m_pt2.y())),
        QPointF(qMax(m_pt1.x(), m_pt2.x()), qMax(m_pt1.y(), m_pt2.y()))));

    //painter->setBrush(Qt::white);
    painter->setPen(QPen(Qt::white, 1.5 * k));
    painter->drawLine(line);

    line.setLength(20.0 * k);
    line.setAngle(angle + 10);
    painter->drawLine(line);
    line.setAngle(angle - 10);
    painter->drawLine(line);

    QFont font;
    font.setPixelSize(15);
    painter->setFont(font);
    painter->translate(m_pt2);
    painter->scale(k, -k);
    painter->drawText(m_textRect, Qt::AlignLeft, m_text);
}

void Ruler::setPoint1(const QPointF& point1)
{
    m_pt1 = point1;
}

void Ruler::setPoint2(const QPointF& point2)
{
    m_pt2 = point2;
    QLineF line(m_pt2, m_pt1);
    const double width = m_pt1.x() > m_pt2.x() ? m_pt1.x() - m_pt2.x() : m_pt2.x() - m_pt1.x();
    const double height = m_pt1.y() > m_pt2.y() ? m_pt1.y() - m_pt2.y() : m_pt2.y() - m_pt1.y();
    const double length = line.length();
    m_text = "    width = " + QString::number(width) + " mm\n";
    m_text.append("    height = " + QString::number(height) + " mm\n");
    m_text.append("    length = " + QString::number(length) + " mm");

    QFont font;
    font.setPixelSize(15);
    m_textRect = QFontMetricsF(font).boundingRect(QRectF(), Qt::AlignLeft, m_text);

    update();
}

int Ruler::type() const { return RulerType; }
