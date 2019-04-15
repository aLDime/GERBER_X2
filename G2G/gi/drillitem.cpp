#include "drillitem.h"
#include "excellon/exvars.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <graphicsview.h>

using namespace ClipperLib;
using namespace Excellon;

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
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
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

const File* DrillItem::file() const
{
    if (m_hole)
        return m_hole->file;
    return nullptr;
}

Paths DrillItem::paths() const
{
    if (m_paths.isEmpty() || m_paths.first().isEmpty()) {
        Path path;
        if (m_hole)
            path = CirclePath(m_diameter * uScale, toIntPoint(m_hole->state.offsetPos()));
        else
            path = CirclePath(m_diameter * uScale, toIntPoint(pos()));
        ReversePath(path);
        return { path };
    }
    return m_paths;
}

void DrillItem::updateHole()
{
    if (!m_hole)
        return;

    setToolTip(QString("Tool %1, Ø%2mm").arg(m_hole->state.tCode).arg(m_diameter));
    m_paths = { toPath(m_hole->state.path.translated(m_hole->state.format->offsetPos)) };
    m_diameter = m_hole->state.currentToolDiameter();
    create();
    update(m_rect);
}

void DrillItem::create()
{
    m_shape = QPainterPath();
    if (m_paths.isEmpty() || m_paths.first().isEmpty()) {
        if (m_hole) {
            setToolTip(QString("Tool %1, Ø%2mm").arg(m_hole->state.tCode).arg(m_hole->state.currentToolDiameter()));
            m_shape.addEllipse(m_hole->state.offsetPos(), m_diameter / 2, m_diameter / 2);
        } else {
            m_shape.addEllipse(QPointF(), m_diameter / 2, m_diameter / 2);
        }
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
