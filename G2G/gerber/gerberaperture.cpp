#include "gerberaperture.h"

GerberAperture::GerberAperture()
{
    qDebug() << "GerberAperture()";
}

GerberAperture::~GerberAperture()
{
    qDebug() << "~GerberAperture()";
}

Paths GerberAperture::draw(const STATE& state)
{
    if (state.curDCode == D03)
        m_isFlashed = true;
    if (m_size == 0)
        draw();

    Paths tmpPpaths;
    for (Path var : m_paths) {
        if (state.imgPolarity == NEGATIVE) {
            ReversePath(var);
        }
        if (state.format.unitMode == INCHES && type() == APERTURE_MACRO) {
            for (IntPoint& pt : var) {
                pt.X *= 25.4;
                pt.Y *= 25.4;
            }
        }
        if (state.curPos.X != 0 || state.curPos.Y != 0) {
            translate(var, state.curPos);
        }
        tmpPpaths.push_back(var);
    }
    return tmpPpaths;
}

double GerberAperture::size()
{
    if (m_size == 0)
        draw();
    return m_size;
}

Path GerberAperture::drawDrill(const STATE& state)
{
    if (qFuzzyIsNull(m_drillDiam))
        return Path();

    Path drill = circle(m_drillDiam * uScale);

    if (state.imgPolarity == POSITIVE)
        ReversePath(drill);

    translate(drill, state.curPos);
    return drill;
}

Path GerberAperture::circle(double diametr, IntPoint center)
{
    Path poligon(STEPS_PER_CIRCLE);
    double radius = diametr / 2.0;
    for (int i = 0; i < STEPS_PER_CIRCLE; ++i) {
        poligon[i] = IntPoint(
            (qCos(qDegreesToRadians((double)i * 360.0 / STEPS_PER_CIRCLE)) * radius) + center.X,
            (qSin(qDegreesToRadians((double)i * 360.0 / STEPS_PER_CIRCLE)) * radius) + center.Y);
    }
    if (Area(poligon) < 0) {
        ReversePath(poligon);
    }
    return poligon;
}

Path GerberAperture::rect(double width, double height, IntPoint center)
{
    Path poligon(4);
    double halfWidth = width * 0.5;
    double halfHeight = height * 0.5;
    poligon[0] = IntPoint(-halfWidth + center.X, +halfHeight + center.Y);
    poligon[1] = IntPoint(-halfWidth + center.X, -halfHeight + center.Y);
    poligon[2] = IntPoint(+halfWidth + center.X, -halfHeight + center.Y);
    poligon[3] = IntPoint(+halfWidth + center.X, +halfHeight + center.Y);
    //    poligon[4] = IntPoint(-halfWidth + center.X, +halfHeight + center.Y);
    if ((Area(poligon) < 0.0)) {
        ReversePath(poligon);
    }
    return poligon;
}

Path& GerberAperture::rotate(Path& poligon, double angle, IntPoint center)
{
    double tmpAangle;
    double length;
    bool fl = Area(poligon) < 0;
    for (int i = 0; i < poligon.size(); ++i) {
        tmpAangle = Angle(center, poligon[i]);
        length = Length(center, poligon[i]);
        poligon[i] = IntPoint(qCos(qDegreesToRadians(tmpAangle + angle)) * length, qSin(qDegreesToRadians(tmpAangle + angle)) * length);
    }
    if (fl != (Area(poligon) < 0)) {
        ReversePath(poligon);
    }
    return poligon;
}

void GerberAperture::translate(Path& path, IntPoint pos)
{
    if (pos.X == 0 && pos.Y == 0)
        return;
    for (Path::size_type i = 0, size = path.size(); i < size; ++i) {
        path[i].X += pos.X;
        path[i].Y += pos.Y;
    }
}
