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
    m_font.setPixelSize(16);
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

    painter->save();
    painter->setBrush(QColor(127, 127, 127, 100));
    painter->setPen(QPen(Qt::green, 0.0)); //1.5 * k));
    painter->setRenderHint(QPainter::Antialiasing, false);
    // draw rect
    painter->drawRect(QRectF(
        QPointF(qMin(m_pt1.x(), m_pt2.x()), qMin(m_pt1.y(), m_pt2.y())),
        QPointF(qMax(m_pt1.x(), m_pt2.x()), qMax(m_pt1.y(), m_pt2.y()))));

    // draw cross
    const double length = 20.0 / GraphicsView ::self->matrix().m11();
    painter->drawLine(QLineF::fromPolar(length, 0.000).translated(m_pt1));
    painter->drawLine(QLineF::fromPolar(length, 90.00).translated(m_pt1));
    painter->drawLine(QLineF::fromPolar(length, 180.0).translated(m_pt1));
    painter->drawLine(QLineF::fromPolar(length, 270.0).translated(m_pt1));
    painter->drawLine(QLineF::fromPolar(length, 0.000).translated(m_pt2));
    painter->drawLine(QLineF::fromPolar(length, 90.00).translated(m_pt2));
    painter->drawLine(QLineF::fromPolar(length, 180.0).translated(m_pt2));
    painter->drawLine(QLineF::fromPolar(length, 270.0).translated(m_pt2));
    painter->restore();

    // draw arrow
    painter->setPen(QPen(Qt::white, 0.0));
    painter->drawLine(line);
    line.setLength(20.0 * k);
    line.setAngle(angle + 10);
    painter->drawLine(line);
    line.setAngle(angle - 10);
    painter->drawLine(line);

    // draw text
    painter->setFont(m_font);
    painter->translate(m_pt2);
    painter->scale(k, -k);
    painter->drawText(m_textRect, Qt::AlignLeft, m_text);

    //    QPainterPath path;
    //    path.addText(0.0, 0.0, m_font, m_text);
    //    painter->setPen(Qt::NoPen);
    //    painter->setBrush(Qt::white);
    //    for (const QPolygonF& poly : path.toFillPolygons()) {
    //        painter->drawPolygon(poly);
    //    }
}

void Ruler::setPoint1(const QPointF& point1)
{
    m_pt1 = point1;
}

void Ruler::setPoint2(const QPointF& point2)
{
    m_pt2 = point2;
    QLineF line(m_pt1, m_pt2);
    const double width = m_pt1.x() > m_pt2.x() ? m_pt1.x() - m_pt2.x() : m_pt2.x() - m_pt1.x();
    const double height = m_pt1.y() > m_pt2.y() ? m_pt1.y() - m_pt2.y() : m_pt2.y() - m_pt1.y();
    const double length = line.length();
    m_text = QString("    ∆X = %1 mm\n"
                     "    ∆Y = %2 mm\n"
                     "    ∆ / = %3 mm")
                 .arg(width, 4, 'f', 3, '0')
                 .arg(height, 4, 'f', 3, '0')
                 .arg(length, 4, 'f', 3, '0');
    m_textRect = QFontMetricsF(m_font).boundingRect(QRectF(), Qt::AlignLeft, m_text);
    update();
}

int Ruler::type() const { return RulerType; }
