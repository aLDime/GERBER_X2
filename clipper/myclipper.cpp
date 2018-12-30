#include "myclipper.h"

Path toPath(const QPolygonF& p)
{
    Path path;
    for (const QPointF& pt : p)
        path.push_back(IntPoint(pt.x() * uScale, pt.y() * uScale));
    return path;
}

Paths toPaths(const QVector<QPolygonF>& p)
{
    Paths paths;
    for (const QPolygonF& pl : p)
        paths.push_back(toPath(pl));
    return paths;
}

QPolygonF toQPolygon(const Path& p)
{
    QPolygonF polygon;
    for (const IntPoint& pt : p)
        polygon.push_back(QPointF(pt.X * dScale, pt.Y * dScale));
    return polygon;
}

QVector<QPolygonF> toQPolygons(const Paths& p)
{
    QVector<QPolygonF> polygons;
    for (const Path& pl : p)
        polygons.push_back(toQPolygon(pl));
    return polygons;
}

QPointF toQPointF(const IntPoint& p) { return QPointF(p.X * dScale, p.Y * dScale); }

IntPoint toIntPoint(const QPointF& p) { return IntPoint(p.x() * uScale, p.y() * uScale); }

double Angle(const IntPoint& pt1, const IntPoint& pt2)
{
    const double dx = pt2.X - pt1.X;
    const double dy = pt2.Y - pt1.Y;

    const double theta = atan2(-dy, dx) * 360.0 / M_2PI;

    const double theta_normalized = theta < 0 ? theta + 360 : theta;

    if (qFuzzyCompare(theta_normalized, double(360)))
        return 0.0;
    else
        return theta_normalized;
}

double Length(const IntPoint& pt1, const IntPoint& pt2)
{
    double x = pt2.X - pt1.X;
    double y = pt2.Y - pt1.Y;
    return sqrt(x * x + y * y);
}
