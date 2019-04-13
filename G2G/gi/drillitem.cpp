#include "drillitem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <aperture.h>
#include <gerber.h>
#include <graphicsview.h>

#include "gcode/drl.h"

using namespace ClipperLib;

DrillItem::DrillItem(Hole* hole)
    : m_hole(hole)
    , m_diameter(hole->state.currentToolDiameter())
{
    setAcceptHoverEvents(true);
    //    setCacheMode(DeviceCoordinateCache);
    setFlag(ItemIsSelectable, true);
    m_paths = { toPath(hole->state.path) };
    create();
}

DrillItem::DrillItem(double diameter)
    : m_diameter(diameter)
{
    create();
}

DrillItem::DrillItem(const QPolygonF& path, double diameter)
    : m_diameter(diameter)
{
    m_paths = { toPath(path) };
    create();
}

QRectF DrillItem::boundingRect() const { return m_rect; }

QPainterPath DrillItem::shape() const { return m_shape; }

void DrillItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    //    if (m_paths.first().isEmpty()) {
    //    if (m_penColor)
    //        m_pen.setColor(*m_penColor);
    //    if (m_brushColor)
    //        m_brush.setColor(*m_brushColor);
    painter->save();
    QBrush brush(m_brush);
    if (brush.style() != Qt::SolidPattern) {
        const double scale = 1.0 / GraphicsView::self->matrix().m11();
        brush.setMatrix(QMatrix().scale(scale, scale));
    }

    if (option->state & QStyle::State_Selected)
        brush.setColor(Qt::magenta);

    if (option->state & QStyle::State_MouseOver)
        brush.setColor(brush.color().dark(150));

    painter->setBrush(brush);
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

    create();
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
    if (m_paths.isEmpty() || m_paths.first().isEmpty()) {
        Path poligon(CirclePath(m_diameter * uScale, toIntPoint(pos())));
        ReversePath(poligon);
        return { poligon };
    }
    return m_paths;
}

void DrillItem::updateHole()
{
    if (!m_hole)
        return;

    State& state = m_hole->state;
    setToolTip(QString("Tool %1, Ø%2mm").arg(state.tCode).arg(m_diameter));

    if (state.path.isEmpty())
        setPos(state.pos + state.format->offsetPos);
    else
        m_paths = { toPath(state.path.translated(state.format->offsetPos)) };

    m_diameter = state.currentToolDiameter();
    create();
    update(m_rect);
}

void DrillItem::create()
{
    m_shape = QPainterPath();
    if (m_paths.isEmpty() || m_paths.first().isEmpty()) {
        if (m_hole) {
            State& state = m_hole->state;
            setPos(state.pos + state.format->offsetPos);
            setToolTip(QString("Tool %1, Ø%2mm").arg(state.tCode).arg(state.currentToolDiameter()));
        }
        m_shape.addEllipse(QPointF(), m_diameter / 2, m_diameter / 2);
        m_rect = m_shape.boundingRect();
    } else {
        Paths tmpPpath;
        ClipperOffset offset;
        offset.AddPath(m_paths.first(), jtRound, etOpenRound);
        offset.Execute(tmpPpath, m_diameter * 0.5 * uScale);
        for (Path& path : tmpPpath) {
            path.append(path.first());
            m_shape.addPolygon(toQPolygon(path));
        }
        m_rect = m_shape.boundingRect();
    }
}
