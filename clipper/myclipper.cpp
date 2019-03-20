#include "myclipper.h"
#include <QElapsedTimer>

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

bool PointOnPolygon(const IntPoint& pt, const Path& path)
{
    //returns 0 if false, +1 if true, -1 if pt ON polygon boundary
    int cnt = path.size();
    if (cnt < 3)
        return 0;
    IntPoint ip = path[0];

    for (int i = 1; i <= cnt; ++i) {
        IntPoint ipNext(i == cnt ? path[0] : path[i]);
        const double k = 100;
        const double l1 = Length(pt, ip);
        const double l2 = Length(pt, ipNext);
        const double l3 = Length(ip, ipNext);
        if (l1 < k) {
            return true;
        } else if (l2 < k) {
            return true;
        } else if (l1 < l3 && l2 < l3) {
            const double p = (l1 + l2 + l3) * 0.5;
            const double h = (2 / l3) * sqrt(p * (p - l1) * (p - l2) * (p - l3));
            if (h < k) {
                return true;
            }
        }
        ip = ipNext;
    }
    return false;
}
