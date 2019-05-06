#include "termalpreviewitem.h"
#include "tooldatabase/tool.h"
#include <QPainter>
#include <gbrfile.h>

extern Paths offset(const Path path, double offset, bool fl = false);

QPainterPath TermalPreviewItem::drawApetrure(const Gerber::GraphicObject& go, int id)
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

QPainterPath TermalPreviewItem::drawPoly(const Gerber::GraphicObject& go)
{
    QPainterPath painterPath;
    for (QPolygonF& polygon : toQPolygons(go.paths /* go.gFile->apertures()->value(id)->draw(go.state)*/)) {
        polygon.append(polygon.first());
        painterPath.addPolygon(polygon);
    }
    //    const double hole = go.gFile->apertures()->value(id)->drillDiameter() * 0.5;
    //    if (hole)
    //        painterPath.addEllipse(toQPointF(go.state.curPos()), hole, hole);
    return painterPath;
}

//TermalPreviewItem::TermalPreviewItem(const Gerber::GraphicObject& go, int id)
//    : id(id)
//    , grob(&go)
//    , m_sourcePath(drawApetrure(go, id))
//    , m_type(Termal)
//    , m_pen(Qt::darkGray, 0.0)
//    , m_brush(Qt::darkGray)
//{
//    setFlag(ItemIsSelectable, true);
//}

TermalPreviewItem::TermalPreviewItem(const Gerber::GraphicObject& go, Tool& tool)
    : tool(tool)
    , grob(&go)
    , m_sourcePath(drawPoly(go))
    , m_type(Termal)
    , m_pen(Qt::darkGray, 0.0)
    , m_brush(Qt::darkGray)
{
    setFlag(ItemIsSelectable, true);
    redraw();
}

TermalPreviewItem::~TermalPreviewItem()
{
    qDebug("~TermalPreviewItem()");
}

void TermalPreviewItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    // draw source
    if (isSelected()) {
        m_pen.setColor(Qt::green);
        m_brush.setColor(Qt::green);
    } else {
        m_pen.setColor(Qt::darkGray);
        m_brush.setColor(Qt::darkGray);
    }
    painter->setPen(m_pen);
    painter->setBrush(m_brush);
    painter->drawPath(m_sourcePath);
    // draw hole
    if (tool.isValid()) {
        //item->setBrush(QBrush(Qt::red, Qt::Dense4Pattern));
        //painter->setPen(QPen(Qt::red, 1.5 / scene()->views().first()->matrix().m11()));
        if (isSelected())
            painter->setPen(m_pen);
        else
            painter->setPen(QPen(Qt::red, 0.0));
        painter->setBrush(QBrush(QColor(255, 0, 0, 100)));
        painter->drawPath(m_toolPath);
    }
}

QRectF TermalPreviewItem::boundingRect() const { return m_sourcePath.boundingRect().united(m_toolPath.boundingRect()); }

int TermalPreviewItem::type() const { return m_type; }

//void TermalPreviewItem::setToolId(int toolId)
//{
//    id = toolId;
//    //    if (m_toolId > -1) {
//    //        m_toolPath = QPainterPath();
//    //        const double diameter = ToolHolder::tools[m_toolId].diameter;
//    //        switch (m_type) {
//    //            //        case Slot: {
//    //            //            Paths tmpPpath;
//    //            //            ClipperOffset offset;
//    //            //            offset.AddPath(hole->item->paths().first(), jtRound, etOpenRound);
//    //            //            offset.Execute(tmpPpath, diameter * 0.5 * uScale);
//    //            //            for (Path& path : tmpPpath) {
//    //            //                path.append(path.first());
//    //            //                m_toolPath.addPolygon(toQPolygon(path));
//    //            //            }
//    //            //            Path path(hole->item->paths().first());
//    //            //            if (path.size()) {
//    //            //                for (IntPoint& pt : path) {
//    //            //                    m_toolPath.moveTo(toQPointF(pt) - QPointF(0.0, diameter * 0.7));
//    //            //                    m_toolPath.lineTo(toQPointF(pt) + QPointF(0.0, diameter * 0.7));
//    //            //                    m_toolPath.moveTo(toQPointF(pt) - QPointF(diameter * 0.7, 0.0));
//    //            //                    m_toolPath.lineTo(toQPointF(pt) + QPointF(diameter * 0.7, 0.0));
//    //            //                }
//    //            //                m_toolPath.moveTo(toQPointF(path.first()));
//    //            //                for (IntPoint& pt : path) {
//    //            //                    m_toolPath.lineTo(toQPointF(pt));
//    //            //                }
//    //            //            }
//    //            //        } break;
//    //            //        case Drill:
//    //            //            m_toolPath.addEllipse(hole->state.offsetedPos(), diameter * 0.5, diameter * 0.5);
//    //            //            m_toolPath.moveTo(hole->state.offsetedPos() - QPointF(0.0, diameter * 0.7));
//    //            //            m_toolPath.lineTo(hole->state.offsetedPos() + QPointF(0.0, diameter * 0.7));
//    //            //            m_toolPath.moveTo(hole->state.offsetedPos() - QPointF(diameter * 0.7, 0.0));
//    //            //            m_toolPath.lineTo(hole->state.offsetedPos() + QPointF(diameter * 0.7, 0.0));
//    //            //            break;
//    //        case Apetrure:
//    //            m_toolPath.addEllipse(toQPointF(grob->state.curPos()), diameter * 0.5, diameter * 0.5);
//    //            m_toolPath.moveTo(toQPointF(grob->state.curPos()) - QPointF(0.0, diameter * 0.7));
//    //            m_toolPath.lineTo(toQPointF(grob->state.curPos()) + QPointF(0.0, diameter * 0.7));
//    //            m_toolPath.moveTo(toQPointF(grob->state.curPos()) - QPointF(diameter * 0.7, 0.0));
//    //            m_toolPath.lineTo(toQPointF(grob->state.curPos()) + QPointF(diameter * 0.7, 0.0));
//    //            break;
//    //        }
//    //    }
//    redraw();
//    update();
//}

IntPoint TermalPreviewItem::pos() const
{
    return grob->state.curPos();
}

Paths TermalPreviewItem::paths() const
{
    return grob->paths;
}

void TermalPreviewItem::redraw()
{
    Paths paths;
    ClipperOffset offset;
    offset.AddPaths(grob->paths, jtRound, etClosedPolygon);
    offset.Execute(paths, tool.diameter * uScale * 0.5);

    Clipper clipper;
    //    clipper.AddPaths(paths, ptSubject, true);
    //    clipper.AddPaths(grob->paths, ptSubject, true);
    //    clipper.Execute(ctXor, paths);
    //    clipper.Clear();
    clipper.AddPaths(paths, ptSubject, false);
    IntPoint center(toIntPoint(m_sourcePath.boundingRect().center()));
    double radius = (m_sourcePath.boundingRect().width() + m_sourcePath.boundingRect().height()) * uScale * 0.5;

    for (int i = 0; i < m_count; ++i) {
        ClipperOffset offset;
        Path path{
            center, IntPoint((cos(i * 2 * M_PI / m_count + qDegreesToRadians(m_angle)) * radius) + center.X, (sin(i * 2 * M_PI / m_count + qDegreesToRadians(m_angle)) * radius) + center.Y)
        };

        offset.AddPath(path, jtSquare, etOpenSquare);
        Paths paths;
        offset.Execute(paths, (m_tickness + tool.diameter) * uScale * 0.5);
        clipper.AddPath(paths.first(), ptClip, true);
    }
    {

        PolyTree polytree;
        clipper.Execute(ctDifference, polytree, pftPositive);
        PolyTreeToPaths(polytree, paths);
    }
    {
        ClipperOffset offset;
        offset.AddPaths(paths, jtRound, etOpenRound);
        offset.Execute(paths, tool.diameter * uScale * 0.5);
    }

    m_toolPath = QPainterPath();
    for (QPolygonF& polygon : toQPolygons(paths)) {
        polygon.append(polygon.first());
        m_toolPath.addPolygon(polygon);
    }
    //    m_toolPath.addEllipse(toQPointF(grob->state.curPos()), diameter * 0.5, diameter * 0.5);
    //    m_toolPath.moveTo(toQPointF(grob->state.curPos()) - QPointF(0.0, diameter * 0.7));
    //    m_toolPath.lineTo(toQPointF(grob->state.curPos()) + QPointF(0.0, diameter * 0.7));
    //    m_toolPath.moveTo(toQPointF(grob->state.curPos()) - QPointF(diameter * 0.7, 0.0));
    //    m_toolPath.lineTo(toQPointF(grob->state.curPos()) + QPointF(diameter * 0.7, 0.0));
    update();
}

double TermalPreviewItem::angle() const
{
    return m_angle;
}

void TermalPreviewItem::setAngle(double angle)
{
    m_angle = angle;
    redraw();
}

double TermalPreviewItem::tickness() const
{
    return m_tickness;
}

void TermalPreviewItem::setTickness(double tickness)
{
    m_tickness = tickness;
    redraw();
}

int TermalPreviewItem::count() const
{
    return m_count;
}

void TermalPreviewItem::setCount(int count)
{
    m_count = count;
    redraw();
}
