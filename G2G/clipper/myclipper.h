#ifndef MYCLIPPER_H
#define MYCLIPPER_H

#include "clipper.hpp"
#include <QPolygonF>

#ifndef M_2PI
#define M_2PI 6.28318530717958647692528676655900576
#endif

using namespace ClipperLib;

typedef QVector /*std::vector*/<Paths> Pathss;

const int uScale = 1000000;
const double dScale = 1.0 / uScale;

Path QPolygonToPath(const QPolygonF& p);
Paths QPolygonsToPaths(const QVector<QPolygonF>& p);

QPolygonF PathToQPolygon(const Path& p);
QVector<QPolygonF> PathsToQPolygons(const Paths& p);

double Angle(const IntPoint& pt1, const IntPoint& pt2);
double Length(const IntPoint& pt1, const IntPoint& pt2);

//IntPoint Center(const IntPoint& pt1, const IntPoint& pt2)
//{
//    return IntPoint(int((qint64(pt1.X) + pt2.X) / 2), int((qint64(pt1.Y) + pt2.Y) / 2));
//}

#endif // MYCLIPPER_H
