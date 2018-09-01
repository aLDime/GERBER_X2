#include "gcode.h"

#include "forms/materialsetupform.h"
#include <QDebug>
#include <QFile>
#include <QPainter>
#include <QSettings>
#include <QStyleOptionGraphicsItem>
#include <QtMath>

///////////////////////////////////////////////
/// \brief GCodeProfile::GCodeProfile
/// \param paths
/// \param tool
/// \param m_depth
///
GCodeProfile::GCodeProfile(const Paths& paths, const Tool& tool, double m_depth)
    : paths(paths)
    , tool(tool)
    , m_depth(m_depth)
    , d(tool.diameter)
{

    //    QGraphicsPolygonItem* polygonItem;

    //    for (const Path& path : paths) {
    //        polygonItem = new QGraphicsPolygonItem(PathToQPolygon(path));
    //        polygonItem->setPen(QPen(QColor(150, 150, 150), d, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    //        polygonItem->setBrush(Qt::NoBrush);
    //        append(polygonItem);
    //    }
    //QPainterPath ppath;
    int i{ 0 };
    for (const Path& path : paths) {
        //ppath.addPolygon(PathToQPolygon(path));
        //        polygonItem = new QGraphicsPolygonItem(PathToQPolygon(path));
        //        polygonItem->setPen(QPen(Qt::white, 0.0));
        //        polygonItem->setBrush(Qt::NoBrush);
        GItem* item = new GItem(path);
        item->setPen(QPen(QColor::fromHsvF((0.83333 / paths.size()) * i++, 1.0, 1.0, 0.5), tool.getDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        append(item);
    }
    //setPath(ppath);
    //setPen(QPen(Qt::white, tool.getDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    setBrush(Qt::NoBrush);
}

Paths GCodeProfile::getPaths() const { return paths; }

void GCodeProfile::save(const QString& name)
{
    QList<QString> sl;
    QVector<QPolygonF> paths(PathsToQPolygons(paths));

    sl.append("G17"); //XY
    sl.append(QString("G0Z%1").arg(MaterialSetupForm::z, 0, 'f', 3)); //HomeZ
    sl.append(QString("G0X%1Y%2S%3M3").arg(MaterialSetupForm::homePos.x(), 0, 'f', 3).arg(MaterialSetupForm::homePos.y(), 0, 'f', 3).arg(tool.spindleSpeed)); //HomeXY
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
    sl.append(QString("G0X%1Y%2").arg(MaterialSetupForm::homePos.x(), 0, 'f', 3).arg(MaterialSetupForm::homePos.y(), 0, 'f', 3));
    sl.append("M30");

    QFile file(name);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (QString& s : sl) {
            out << s << endl;
        }
    }
}
GItem::GItem(const Path& path)
    : m_path(path)
{
    m_brush = QBrush(Qt::NoBrush);
    if (m_path.first() != m_path.last())
        m_path.append(m_path.first());
    m_shape.addPolygon(PathToQPolygon(m_path));

    rect = m_shape.boundingRect();
    //setAcceptHoverEvents(true);
    //setFlag(ItemIsSelectable, true);
}

QRectF GItem::boundingRect() const
{
    double k = m_pen.widthF() / 2;
    return rect + QMarginsF(k, k, k, k);
}

//QPainterPath GItem::shape() const { return m_shape; }

void GItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    //QBrush brush(m_brush);
    //QColor c(m_brush.color());
    //if (option->state & QStyle::State_Selected) {
    //    c.setAlpha(255);
    ///    brush.setColor(c);
    //    c.setAlpha(100);
    //    pen = QPen(c, 0.0);
    //}
    //if (option->state & QStyle::State_MouseOver) {
    //    //        c.setAlpha(200);
    //    //        brush.setColor(c);
    //    c.setAlpha(255);
    //    pen = QPen(c, 0.0);
    //}
    painter->setBrush(m_brush);
    painter->setPen(m_pen);
    painter->drawPath(m_shape);
    painter->setPen(QPen(Qt::white, 0.0));
    painter->drawPath(m_shape);
}

int GItem::type() const { return QGraphicsItem::UserType + 3; }
