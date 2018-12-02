#include "drillitem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <gerber.h>

using namespace ClipperLib;

DrillItem::DrillItem(double diameter, DrillFile* file)
    : m_diameter(diameter)
    , m_file(file)
{
    //            IntPoint center(hole.state.pos.x() * uScale, hole.state.pos.y() * uScale);
    //            double radius = hole.state.currentToolDiameter * uScale / 2.0;
    //            Path poligon(G::STEPS_PER_CIRCLE);
    //            for (int i = 0; i < G::STEPS_PER_CIRCLE; ++i) {
    //                poligon[i] = IntPoint(
    //                    (qCos(i * M_2PI / G::STEPS_PER_CIRCLE) * radius) + center.X,
    //                    (qSin(i * M_2PI / G::STEPS_PER_CIRCLE) * radius) + center.Y);
    //            }
    //            if (Area(poligon) < 0)
    //                ReversePath(poligon);
    setCacheMode(DeviceCoordinateCache);
    m_shape.addEllipse(QPointF(), diameter / 2, diameter / 2);
    m_rect = m_shape.boundingRect();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
}

QRectF DrillItem::boundingRect() const { return m_rect; }

QPainterPath DrillItem::shape() const { return m_shape; }

void DrillItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    painter->save();
    // if (m_pen.width() == 0)
    //painter->setRenderHint(QPainter::Antialiasing, false);
    QColor cb(Qt::white);
    if (option->state & QStyle::State_Selected)
        cb = QColor(Qt::black);

    if (option->state & QStyle::State_MouseOver)
        cb = QColor(Qt::red);

    painter->setBrush(cb);
    painter->setPen(QPen(Qt::white, 0.0));
    painter->drawPath(m_shape);
    painter->restore();
}

int DrillItem::type() const { return DrillItemType; }

double DrillItem::diameter() const { return m_diameter; }

void DrillItem::setDiameter(double diameter)
{
    if (m_diameter == diameter)
        return;
    m_diameter = diameter;
    QPainterPath path;
    path.addEllipse(QPointF(), diameter / 2, diameter / 2);
    m_shape = path;
    m_rect = m_shape.boundingRect();
    update(m_rect);
}

const DrillFile* DrillItem::file() const { return m_file; }

Paths DrillItem::paths() const
{
    if (m_paths.isEmpty()) {
        IntPoint center(pos().x() * uScale, pos().y() * uScale);
        double radius = m_diameter * uScale / 2;
        Path poligon(G::StepsPerCircle);
        for (int i = 0; i < G::StepsPerCircle; ++i) {
            poligon[i] = IntPoint(
                (qCos(i * M_2PI / G::StepsPerCircle) * radius) + center.X,
                (qSin(i * M_2PI / G::StepsPerCircle) * radius) + center.Y);
        }
        if (Area(poligon) > 0)
            ReversePath(poligon);
        m_paths.append(poligon);
    }
    return m_paths;
}
