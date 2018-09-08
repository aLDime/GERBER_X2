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
    switch (type) {
    case PROFILE:
    case POCKET:
        save2(name);
        break;
    case DRILLING:
        save1(name);
        break;
    case MATERIAL_SETUP_FORM:
    default:
        break;
    }
}

void GCode::save1(const QString& name)
{
    QList<QString> sl;
    QPolygonF path(PathToQPolygon(paths.first()));

    sl.append("G17"); //XY
    sl.append(QString("G0Z%1").arg(MaterialSetupForm::z, 0, 'f', 3)); //HomeZ
    QPointF home(MaterialSetupForm::homePos);
    home -= MaterialSetupForm::zeroPos;
    sl.append(QString("G0X%1Y%2S%3M3").arg(home.x(), 0, 'f', 3).arg(home.y(), 0, 'f', 3).arg(tool.spindleSpeed)); //HomeXY
    for (QPointF& point : path) {
        point -= MaterialSetupForm::zeroPos;
        sl.append(QString("G0X%1Y%2").arg(point.x(), 0, 'f', 3).arg(point.y(), 0, 'f', 3)); //start xy
        sl.append(QString("G0Z%1").arg(MaterialSetupForm::plunge, 0, 'f', 3)); //start z
        sl.append(QString("G1Z%1F%2").arg(-m_depth, 0, 'f', 3).arg(tool.plungeRate, 0, 'f', 3)); //start z
        sl.append(QString("G0Z%1").arg(MaterialSetupForm::clearence, 0, 'f', 3));
    }
    sl.append(QString("G0Z%1").arg(MaterialSetupForm::z, 0, 'f', 3));
    sl.append(QString("G0X%1Y%2").arg(home.x(), 0, 'f', 3).arg(home.y(), 0, 'f', 3));
    sl.append("M30");

    QFile file(name);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (QString& s : sl) {
            out << s << endl;
        }
    }
}

void GCode::save2(const QString& name)
{
    QList<QString> sl;
    QVector<QPolygonF> paths(PathsToQPolygons(paths));

    sl.append("G17"); //XY
    sl.append(QString("G0Z%1").arg(MaterialSetupForm::z, 0, 'f', 3)); //HomeZ
    QPointF home(MaterialSetupForm::homePos);
    home -= MaterialSetupForm::zeroPos;
    sl.append(QString("G0X%1Y%2S%3M3").arg(home.x(), 0, 'f', 3).arg(home.y(), 0, 'f', 3).arg(tool.spindleSpeed)); //HomeXY
    QPointF lastPoint;
    for (QPolygonF& path : paths) {
        QPointF point(path.last());
        point -= MaterialSetupForm::zeroPos;
        sl.append(QString("G0X%1Y%2").arg(point.x(), 0, 'f', 3).arg(point.y(), 0, 'f', 3)); //start xy
        sl.append(QString("G0Z%1").arg(MaterialSetupForm::plunge, 0, 'f', 3)); //start z
        sl.append(QString("G1Z%1F%2").arg(-m_depth, 0, 'f', 3).arg(tool.plungeRate, 0, 'f', 3)); //start z
        bool fl = true;
        for (QPointF& point : path) {
            QString str("G1");
            point -= MaterialSetupForm::zeroPos;
            if (lastPoint.x() != point.x())
                str.append(QString("X%1").arg(point.x(), 0, 'f', 3));
            if (lastPoint.y() != point.y())
                str.append(QString("Y%1").arg(point.y(), 0, 'f', 3));
            if (fl) {
                str.append(QString("F%1").arg(tool.feedRate, 0, 'f', 3));
                fl = false;
            }
            sl.append(str);
            lastPoint = point;
        }
        sl.append(QString("G0Z%1").arg(MaterialSetupForm::clearence, 0, 'f', 3));
    }
    sl.append(QString("G0Z%1").arg(MaterialSetupForm::z, 0, 'f', 3));
    sl.append(QString("G0X%1Y%2").arg(home.x(), 0, 'f', 3).arg(home.y(), 0, 'f', 3));
    sl.append("M30");

    QFile file(name);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (QString& s : sl) {
            out << s << endl;
        }
    }
}

QString GCode::name() const
{
    return m_name;
}

void GCode::setName(const QString& name)
{
    m_name = name;
}
////////////////////////////////////////////////////
/// \brief PathItem::PathItem
/// \param path
///
PathItem::PathItem(const Path& path)
    : m_path(path)
{
    //    if (m_path.first() != m_path.last())
    //        m_path.append(m_path.first());
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
    diameter /= 2;
    m_shape.addEllipse(QPointF(), diameter, diameter);
    rect = m_shape.boundingRect();
}

QRectF DrillItem::boundingRect() const { return rect; }

QPainterPath DrillItem::shape() const { return m_shape; }

void DrillItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setBrush(m_brush);
    painter->setPen(m_pen);
    painter->drawPath(m_shape);
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
