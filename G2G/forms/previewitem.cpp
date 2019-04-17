#include "previewitem.h"
#include <QPainter>
#include <exfile.h>
#include <gbrfile.h>

extern Paths offset(const Path path, double offset, bool fl = false);

QPainterPath PreviewItem::drawApetrure(const Gerber::GraphicObject& go, int id)
{
    QPainterPath painterPath;
    for (QPolygonF& polygon : toQPolygons(go.paths /* go.gFile->apertures()->value(id)->draw(go.state)*/)) {
        polygon.append(polygon.first());
        painterPath.addPolygon(polygon);
    }
    const double hole = go.gFile->apertures()->value(id)->drillDiameter() * 0.5;
    if (hole)
        painterPath.addEllipse(toQPointF(go.state.curPos()), hole, hole);
    return painterPath;
}

QPainterPath PreviewItem::drawDrill(const Excellon::Hole& hole)
{
    QPainterPath painterPath;
    painterPath.addEllipse(hole.state.offsetPos(), hole.state.currentToolDiameter() * 0.5, hole.state.currentToolDiameter() * 0.5);
    return painterPath;
}

QPainterPath PreviewItem::drawSlot(const Excellon::Hole& hole)
{
    QPainterPath painterPath;
    for (Path& path : offset(hole.item->paths().first(), hole.state.currentToolDiameter()))
        painterPath.addPolygon(toQPolygon(path));
    return painterPath;
}

PreviewItem::PreviewItem(const Gerber::GraphicObject& go, int id)
    : id(id)
    , grob(&go)
    , m_sourcePath(drawApetrure(go, id))
    , m_sourceDrill(go.gFile->apertures()->value(id)->drillDiameter() ? go.gFile->apertures()->value(id)->drillDiameter() : go.gFile->apertures()->value(id)->apSize())
    , m_type(Apetrure)
    , m_pen(Qt::darkGray, 0.0)
    , m_brush(Qt::darkGray)
{
}

PreviewItem::PreviewItem(const Excellon::Hole& hole, const QPolygonF& toolPath)
    : hole(&hole)
    , m_sourcePath(drawSlot(hole))
    , m_sourceDrill(hole.state.currentToolDiameter())
    , m_toolPath(toolPath)
    , m_type(Slot)
    , m_pen(Qt::darkGray, 0.0)
    , m_brush(Qt::darkGray)
{
}

PreviewItem::PreviewItem(const Excellon::Hole& hole)
    : hole(&hole)
    , m_sourcePath(drawDrill(hole))
    , m_sourceDrill(hole.state.currentToolDiameter())
    , m_type(Drill)
    , m_pen(Qt::darkGray, 0.0)
    , m_brush(Qt::darkGray)
{
}

PreviewItem::~PreviewItem() {}

void PreviewItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    // draw source
    painter->setPen(m_pen);
    painter->setBrush(m_brush);
    painter->drawPath(m_sourcePath);
    // draw hole
    if (!qFuzzyIsNull(m_currentDrill)) {
        //item->setBrush(QBrush(Qt::red, Qt::Dense4Pattern));
        //painter->setPen(QPen(Qt::red, 1.5 / scene()->views().first()->matrix().m11()));
        if (isSelected)
            painter->setPen(m_pen);
        else
            painter->setPen(QPen(Qt::red, 0.0));
        painter->setBrush(QBrush(QColor(255, 0, 0, 100)));
        painter->drawPath(m_currentPath);
    }
}

QRectF PreviewItem::boundingRect() const { return m_sourcePath.boundingRect().united(m_currentPath.boundingRect()); }

int PreviewItem::type() const { return m_type; }

double PreviewItem::sourceDrill() const { return m_sourceDrill; }

QPolygonF PreviewItem::toolPath() const { return m_toolPath; }

double PreviewItem::currentDrill() const { return m_currentDrill; }

void PreviewItem::setCurrentDrill(double currentDrill)
{
    m_currentDrill = currentDrill;
    if (!qFuzzyIsNull(m_currentDrill)) {
        m_currentPath = QPainterPath();
        switch (m_type) {
        case Slot: {
            Paths tmpPpath;
            ClipperOffset offset;
            offset.AddPath(hole->item->paths().first(), jtRound, etOpenRound);
            offset.Execute(tmpPpath, m_currentDrill * 0.5 * uScale);
            for (Path& path : tmpPpath) {
                path.append(path.first());
                m_currentPath.addPolygon(toQPolygon(path));
            }
            Path path(hole->item->paths().first());
            if (path.size()) {
                for (IntPoint& pt : path) {
                    m_currentPath.moveTo(toQPointF(pt) - QPointF(0.0, m_currentDrill * 0.7));
                    m_currentPath.lineTo(toQPointF(pt) + QPointF(0.0, m_currentDrill * 0.7));
                    m_currentPath.moveTo(toQPointF(pt) - QPointF(m_currentDrill * 0.7, 0.0));
                    m_currentPath.lineTo(toQPointF(pt) + QPointF(m_currentDrill * 0.7, 0.0));
                }
                m_currentPath.moveTo(toQPointF(path.first()));
                for (IntPoint& pt : path) {
                    m_currentPath.lineTo(toQPointF(pt));
                }
            }
        } break;
        case Drill:
            m_currentPath.addEllipse(hole->state.offsetPos(), m_currentDrill * 0.5, m_currentDrill * 0.5);
            m_currentPath.moveTo(hole->state.offsetPos() - QPointF(0.0, m_currentDrill * 0.7));
            m_currentPath.lineTo(hole->state.offsetPos() + QPointF(0.0, m_currentDrill * 0.7));
            m_currentPath.moveTo(hole->state.offsetPos() - QPointF(m_currentDrill * 0.7, 0.0));
            m_currentPath.lineTo(hole->state.offsetPos() + QPointF(m_currentDrill * 0.7, 0.0));
            break;
        case Apetrure:
            m_currentPath.addEllipse(toQPointF(grob->state.curPos()), m_currentDrill * 0.5, m_currentDrill * 0.5);
            m_currentPath.moveTo(toQPointF(grob->state.curPos()) - QPointF(0.0, m_currentDrill * 0.7));
            m_currentPath.lineTo(toQPointF(grob->state.curPos()) + QPointF(0.0, m_currentDrill * 0.7));
            m_currentPath.moveTo(toQPointF(grob->state.curPos()) - QPointF(m_currentDrill * 0.7, 0.0));
            m_currentPath.lineTo(toQPointF(grob->state.curPos()) + QPointF(m_currentDrill * 0.7, 0.0));
            break;
        }
    }
    update();
}

void PreviewItem::setSelected(bool value)
{
    isSelected = value;
    if (isSelected) {
        m_pen.setColor(Qt::green);
        m_brush.setColor(Qt::green);
    } else {
        m_pen.setColor(Qt::darkGray);
        m_brush.setColor(Qt::darkGray);
    }
    update();
}

IntPoint PreviewItem::pos() const
{
    switch (m_type) {
    case Slot:
        return toIntPoint(hole->state.offsetPos());
    case Drill:
        return toIntPoint(hole->state.offsetPos());
    case Apetrure:
        return grob->state.curPos();
    }
    return IntPoint();
}

Paths PreviewItem::paths() const
{
    switch (m_type) {
    case Slot:
        return hole->item->paths();
    case Drill:
        return hole->item->paths();
    case Apetrure: {
        return grob->paths;
    }
    }
    return Paths();
}

bool PreviewItem::fit()
{
    switch (m_type) {
    case Slot:
    case Drill:
        return m_sourceDrill > m_currentDrill;
    case Apetrure:
        return grob->gFile->apertures()->value(id)->fit(m_currentDrill);
    }
    return false;
}
