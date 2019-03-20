#ifndef MYCLIPPER_H
#define MYCLIPPER_H

#include "clipper.hpp"
#include <QPolygonF>

#ifndef M_2PI
#define M_2PI 6.28318530717958647692528676655900576
#endif

using namespace ClipperLib;

typedef QVector /*std::vector*/<Paths> Pathss;

const long long uScale = 100000;
const double dScale = 1.0 / uScale;

Path toPath(const QPolygonF& p);
Paths toPaths(const QVector<QPolygonF>& p);

QPolygonF toQPolygon(const Path& p);
QVector<QPolygonF> toQPolygons(const Paths& p);

QPointF toQPointF(const IntPoint& p);
IntPoint toIntPoint(const QPointF& p);

double Angle(const IntPoint& pt1, const IntPoint& pt2);
double Length(const IntPoint& pt1, const IntPoint& pt2);

bool PointOnPolygon(const IntPoint& pt, const Path& path);
//IntPoint Center(const IntPoint& pt1, const IntPoint& pt2)
//{
//    return IntPoint(int((qint64(pt1.X) + pt2.X) / 2), int((qint64(pt1.Y) + pt2.Y) / 2));
//}

#endif // MYCLIPPER_H
