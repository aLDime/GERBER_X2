#include "gcode.h"

#include "forms/materialsetupform.h"
#include <QFile>
#include <QPainter>
#include <QTextStream>
#include <mygraphicsview.h>
#include <myscene.h>

///////////////////////////////////////////////
/// \brief GCodeProfile::GCodeProfile
/// \param paths
/// \param tool
/// \param m_depth
///
GCode::GCode(const Paths& paths, const Tool& tool, double m_depth, GCodeType type)
    : paths(paths)
    , tool(tool)
    , m_depth(m_depth)
    , type(type)
{
    PathItem* item;
    DrillItem* itemd;
    Path p;
    p.reserve(paths.size());
    switch (type) {
    case PROFILE:
    case POCKET:
        for (const Path& path : paths) {
            item = new PathItem(path);
            //item->setPen(QPen(QColor::fromHsvF((0.83333 / paths.size()) * i++, 1.0, 1.0, 0.5), tool.getDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            item->setPen(QPen(QColor(100, 100, 100), tool.getDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            append(item);
        }

        for (const Path& path : paths) {
            item = new PathItem(path);
            item->setPen(QPen(Qt::black, 0.0, Qt::DashLine));
            //  item->w = d;
            append(item);
            p.append(path.first());
        }
        item = new PathItem(p);
        item->setPen(QPen(Qt::green, 0.0));
        //  item->w = 10;
        append(item);
        //  setPen(QPen(QColor(50, 50, 50), tool.getDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        setBrush(Qt::NoBrush);
        break;
    case DRILLING:
        for (const IntPoint& point : paths.first()) {
            itemd = new DrillItem(tool.diameter);
            itemd->setPos(ToQPointF(point));
            itemd->setPen(QPen(Qt::red, 0.0));
            itemd->setBrush(Qt::red);
            append(itemd);
        }
        item = new PathItem(paths.first());
        item->setPen(QPen(Qt::green, 0.0));
        append(item);
        break;
    case MATERIAL_SETUP_FORM:
        break;
    default:
        break;
    }
    addToTheScene(MyScene ::self);
}

Paths GCode::getPaths() const { return paths; }

void GCode::save(const QString& name)
{
    if (!name.isEmpty())
        m_name = name;
    switch (type) {
    case PROFILE:
    case POCKET:
        saveProfilePocket();
        break;
    case DRILLING:
        saveDrill();
        break;
    case MATERIAL_SETUP_FORM:
    default:
        break;
    }
}

void GCode::saveDrill()
{
    statFile();
    QPolygonF path(PathToQPolygon(paths.first()));

    for (QPointF& point : path)
        point -= MaterialSetupForm::zeroPos;

    for (QPointF& point : path) {
        startPath(point);
        for (int i = 1; m_depth > tool.passDepth * i; ++i) {
            sl.append(g1() + z(-tool.passDepth * i) + feed(tool.plungeRate));
            sl.append(QString("G0Z0"));
        }
        sl.append(g1() + z(-m_depth) + feed(tool.plungeRate));
        endPath();
    }
    endFile();
}

void GCode::saveProfilePocket()
{
    statFile();
    QVector<QPolygonF> paths(PathsToQPolygons(paths));

    for (QPolygonF& path : paths)
        for (QPointF& point : path)
            point -= MaterialSetupForm::zeroPos;

    QPointF lastPoint;

    for (int i = 1; m_depth > tool.passDepth * i; ++i) {
        for (QPolygonF& path : paths) {
            QPointF point(path.last());

            startPath(point);

            sl.append(g1() + z(-tool.passDepth * i) + feed(tool.plungeRate)); //start z

            bool fl = true;
            for (QPointF& point : path) {
                QString str("G1");
                if (lastPoint.x() != point.x())
                    str += x(point.x());
                if (lastPoint.y() != point.y())
                    str += y(point.y());
                if (fl) {
                    str += feed(tool.feedRate);
                    fl = false;
                }
                sl.append(str);
                lastPoint = point;
            }
            endPath();
        }
    }

    for (QPolygonF& path : paths) {
        QPointF point(path.last());

        startPath(point);

        sl.append(g1() + z(-m_depth) + feed(tool.plungeRate)); //start z

        bool fl = true;
        for (QPointF& point : path) {
            QString str("G1");
            if (lastPoint.x() != point.x())
                str += x(point.x());
            if (lastPoint.y() != point.y())
                str += y(point.y());
            if (fl) {
                str += feed(tool.feedRate);
                fl = false;
            }
            sl.append(str);
            lastPoint = point;
        }
        endPath();
    }

    endFile();
}

QString GCode::name() const { return m_name; }

void GCode::setName(const QString& name) { m_name = name; }

void GCode::startPath(const QPointF& point)
{
    sl.append(g0() + x(point.x()) + y(point.y())); //start xy
    sl.append(g0() + z(MaterialSetupForm::plunge)); //start z
}

void GCode::endPath()
{
    sl.append(QString("G0Z%1").arg(format(MaterialSetupForm::clearence)));
}

void GCode::statFile()
{
    sl.clear();
    sl.append("G17"); //XY plane
    sl.append(g0() + z(MaterialSetupForm::z)); //HomeZ

    QPointF home(MaterialSetupForm::homePos - MaterialSetupForm::zeroPos);
    sl.append(g0() + x(home.x()) + y(home.y()) + s(tool.spindleSpeed) + "M3"); //HomeXY
}

void GCode::endFile()
{
    sl.append(g0() + z(MaterialSetupForm::z)); //HomeZ

    QPointF home(MaterialSetupForm::homePos - MaterialSetupForm::zeroPos);
    sl.append(g0() + x(home.x()) + y(home.y()) + s(tool.spindleSpeed) + "M3"); //HomeXY

    sl.append("M30");

    QFile file(m_name);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (QString& s : sl)
            out << s << endl;
    }
    file.close();
}

////////////////////////////////////////////////////
/// \brief PathItem::PathItem
/// \param path
///
PathItem::PathItem(const Path& path)
    : m_path(path)
{
    if (m_path.first() != m_path.last())
        m_path.append(m_path.first());
    m_shape.addPolygon(PathToQPolygon(m_path));
    double k = m_pen.widthF() / 2;
    rect = m_shape.boundingRect() + QMarginsF(k, k, k, k);
}

QRectF PathItem::boundingRect() const
{
    return rect;
}

QPainterPath PathItem::shape() const { return m_shape; }

void PathItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    if (0) {
        painter->setBrush(QBrush(Qt::NoBrush));
        painter->setPen(m_pen);
        painter->setPen(QPen(Qt::white, 0.0));
        painter->drawPath(m_shape);
        //  painter->drawPath(m_shape);
        painter->setBrush(QBrush(Qt::red));
        const double d = m_pen.widthF();
        QRectF r(m_path.first().X * dScale - d / 2, m_path.first().Y * dScale - d / 2, d, d);
        painter->drawEllipse(r);
    } else {
        painter->setBrush(QBrush(Qt::NoBrush));
        painter->setPen(m_pen);
        painter->drawPath(m_shape);
        //  if (w) {
        //  painter->setPen(QPen(Qt::red, 0.0));
        // //  QLineF line(
        // //  QPointF(m_path[0].X * dScale, m_path[0].Y * dScale),
        // //  QPointF(m_path[1].X * dScale, m_path[1].Y * dScale));
        // //  double length = /*w * (*/ 50 / MyGraphicsView::self->matrix().m11() /*)*/;
        // //  double angle = line.angle();
        // //  line.setLength(length);
        // //  painter->drawLine(line);
        // //  line.setAngle(angle + 10);
        // //  painter->drawLine(line);
        // //  line.setAngle(angle - 10);
        // //  painter->drawLine(line);
        // //  for (int i = 0; i < m_path.size() - 1; ++i) {
        // //  QLineF line(
        // //  QPointF(m_path[i].X * dScale, m_path[i].Y * dScale),
        // //  QPointF(m_path[i + 1].X * dScale, m_path[i + 1].Y * dScale));

        // //  double length = /*w * (*/ 50 / MyGraphicsView::self->matrix().m11() /*)*/;
        // //  double angle = line.angle();

        // //  line.setLength(length);
        // //  painter->drawLine(line);

        // //  line.setAngle(angle + 10);
        // //  painter->drawLine(line);

        // //  line.setAngle(angle - 10);
        // //  painter->drawLine(line);
        // // }
        // }
    }
}

int PathItem::type() const { return QGraphicsItem::UserType + 10; }

////////////////////////////////////////////////////
/// \brief PathItem::PathItem
/// \param path
///
DrillItem::DrillItem(double diameter)
    : m_diameter(diameter)
{
    setCacheMode(DeviceCoordinateCache);
    diameter /= 2;
    m_shape.addEllipse(QPointF(), diameter, diameter);
    rect = m_shape.boundingRect();
}

QRectF DrillItem::boundingRect() const { return rect; }

QPainterPath DrillItem::shape() const { return m_shape; }

void DrillItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->save();
    if (m_pen.width() == 0)
        painter->setRenderHint(QPainter::Antialiasing, false);

    painter->setBrush(m_brush);
    painter->setPen(m_pen);
    painter->drawPath(m_shape);

    painter->restore();
}

int DrillItem::type() const { return QGraphicsItem::UserType + 10; }

double DrillItem::diameter() const
{
    return m_diameter;
}

void DrillItem::setDiameter(double diameter)
{
    if (m_diameter == diameter)
        return;
    m_diameter = diameter;
    diameter /= 2;
    QPainterPath path;
    path.addEllipse(QPointF(), diameter, diameter);
    m_shape = path;
    update(rect);
    rect = m_shape.boundingRect();
}
