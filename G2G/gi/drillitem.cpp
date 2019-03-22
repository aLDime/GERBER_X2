#include "drillitem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <aperture.h>
#include <gerber.h>

#include "gcode/drl.h"

using namespace ClipperLib;

DrillItem::DrillItem(Hole* hole)
    : m_hole(hole)
    , m_diameter(hole->state.currentToolDiameter())
{
    create();
}

DrillItem::DrillItem(double diameter)
    : m_diameter(diameter)
{
    create();
}

QRectF DrillItem::boundingRect() const { return m_rect; }

QPainterPath DrillItem::shape() const { return m_shape; }

void DrillItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    if (m_penColor)
        m_pen.setColor(*m_penColor);
    if (m_brushColor)
        m_brush.setColor(*m_brushColor);
    painter->save();
    QColor cb(m_brush.color());
    if (option->state & QStyle::State_Selected) {
        cb = cb.light(150);
        cb = Qt::magenta;
    }

    if (option->state & QStyle::State_MouseOver)
        cb = cb.dark(150);

    painter->setBrush(cb);
    painter->setPen(m_pen);
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

const DrillFile* DrillItem::file() const
{
    if (m_hole)
        return m_hole->file;
    return nullptr;
}

Paths DrillItem::paths() const
{
    if (m_paths.isEmpty()) {
        Path poligon(CirclePath(m_diameter * uScale, toIntPoint(pos())));
        ReversePath(poligon);
        m_paths.append(poligon);
    }
    return m_paths;
}

void DrillItem::updateHole()
{
    if (!m_hole)
        return;

    State& state = m_hole->state;
    setPos(state.pos + state.format->offsetPos);
    setDiameter(state.currentToolDiameter());
    setToolTip(QString("Tool %1, Ø%2mm").arg(state.tCode).arg(m_diameter));
    update(m_rect);
}

void DrillItem::create()
{
    setAcceptHoverEvents(true);
    setCacheMode(DeviceCoordinateCache);
    setFlag(ItemIsSelectable, true);
    if (m_hole) {
        State& state = m_hole->state;
        setPos(state.pos + state.format->offsetPos);
        setToolTip(QString("Tool %1, Ø%2mm").arg(state.tCode).arg(state.currentToolDiameter()));
    }
    m_shape.addEllipse(QPointF(), m_diameter / 2, m_diameter / 2);
    m_rect = m_shape.boundingRect();
}
