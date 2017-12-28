#include "aperture.h"

GerberAperture::GerberAperture()
{
}

GerberAperture::~GerberAperture()
{
    //qDebug() << "~GerberAperture()";
}

Paths GerberAperture::draw(const STATE& state)
{
    if (state.curDCode == D03)
        m_isFlashed = true;
    if (m_size == 0)
        draw();

    Paths tmpPpaths;
    for (Path var : m_paths) {
#ifdef DEPRECATED
        if (state.imgPolarity == NEGATIVE) {
            ReversePath(var);
        }
#endif
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
/////////////////////////////////////////////////////

GACircular::GACircular(double diam, double drillDiam)
{
    m_diam = diam;
    m_drillDiam = drillDiam;
    // GerberAperture interface
}

QString GACircular::name() { return QString("CIRCULAR(D%1)").arg(m_diam); }

APERTURE_TYPE GACircular::type() const { return CIRCULAR; }

void GACircular::draw()
{
    m_paths.push_back(circle(m_diam * uScale));
    m_size = m_diam;
}
/////////////////////////////////////////////////////

GARectangle::GARectangle(double width, double height, double drillDiam)
{
    m_width = width;
    m_height = height;
    m_drillDiam = drillDiam;
}

QString GARectangle::name() { return QString("RECTANGLE(W%1, H%2)").arg(m_width).arg(m_height); }

APERTURE_TYPE GARectangle::type() const { return RECTANGLE; }

void GARectangle::draw()
{
    m_paths.push_back(rect(m_width * uScale, m_height * uScale));
    m_size = qSqrt(m_width * m_width + m_height * m_height);
}
/////////////////////////////////////////////////////

GAObround::GAObround(double width, double height, double drillDiam)
{

    m_width = width;
    m_height = height;
    m_drillDiam = drillDiam;
}

QString GAObround::name() { return QString("OBROUND(W%1, H%2)").arg(m_width).arg(m_height); }

APERTURE_TYPE GAObround::type() const { return OBROUND; }

void GAObround::draw()
{
    Clipper clipper;
    cInt height_ = m_height * uScale;
    cInt width_ = m_width * uScale;
    if (qFuzzyCompare(width_ + 1.0, height_ + 1.0)) {
        m_paths.push_back(circle(width_));
    }
    else {
        if (width_ > height_) {
            clipper.AddPath(circle(height_, IntPoint(-(width_ - height_) / 2, 0)), ptClip, true);
            clipper.AddPath(circle(height_, IntPoint((width_ - height_) / 2, 0)), ptClip, true);
            clipper.AddPath(rect(width_ - height_, height_), ptClip, true);
        }
        else if (width_ < height_) {
            clipper.AddPath(circle(width_, IntPoint(0, -(height_ - width_) / 2)), ptClip, true);
            clipper.AddPath(circle(width_, IntPoint(0, (height_ - width_) / 2)), ptClip, true);
            clipper.AddPath(rect(width_, height_ - width_), ptClip, true);
        }
        clipper.Execute(ctUnion, m_paths, pftNonZero, pftNonZero);
    }
    m_size = qMax(width_, height_);
}
/////////////////////////////////////////////////////

GAPolygon::GAPolygon(double diam, int nVertices, double rotation, double drillDiam)
{
    m_diam = diam;
    m_verticesCount = nVertices;
    m_rotation = rotation;
    m_drillDiam = drillDiam;
}

double GAPolygon::rotation() const { return m_rotation; }

int GAPolygon::verticesCount() const { return m_verticesCount; }

QString GAPolygon::name() { return QString("POLYGON(D%1, N%2)").arg(m_diam).arg(m_verticesCount); }

APERTURE_TYPE GAPolygon::type() const { return POLYGON; }

void GAPolygon::draw()
{
    Path poligon;
    const double step = 360.0 / m_verticesCount;
    const double diam = this->m_diam * uScale;
    for (int i = 0; i < m_verticesCount; ++i) {
        poligon.push_back(IntPoint(qCos(qDegreesToRadians(step * i)) * diam / 2, qSin(qDegreesToRadians(step * i)) * diam / 2));
    }
    if (m_rotation > 0.1) {
        poligon = rotate(poligon, m_rotation);
    }
    m_paths.push_back(poligon);
    m_size = diam;
}
/////////////////////////////////////////////////////

GAMacro::GAMacro(const QString& macro, const QList<QString>& modifiers, const QMap<QString, double>& macroCoefficients)
{
    m_macro = macro;
    m_modifiers = modifiers;
    while (m_modifiers.last().isEmpty()) {
        m_modifiers.removeLast();
    }
    m_macroCoefficients = macroCoefficients;
}

QString GAMacro::name() { return QString("MACRO(%1)").arg(m_macro); }

APERTURE_TYPE GAMacro::type() const { return APERTURE_MACRO; }

void GAMacro::draw()
{
    enum {
        CENTER_LINE = 21,
        CIRCLE = 1,
        COMMENT = 0,
        MOIRE = 6,
        OUTLINE_CUSTOM_POLYGON = 4, // MAXIMUM 5000 POINTS
        OUTLINE_REGULAR_POLYGON = 5, // 3-12 POINTS
        THERMAL = 7,
        VECTOR_LINE = 20,
    };

    Clipper clipper;
    int exposure;
    QList<double> mod;
    QList<QString> stringList;
    IntPoint center;
    Path polygon;
    QVector<QPair<int, Paths> > items;
    try {
        for (int i = 0; i < m_modifiers.size(); ++i) {
            QString var = m_modifiers[i];
            mod.clear();
            if (var.at(0) == '0') {
                qDebug() << "Macro comment:" << var;
                continue;
            }
            if (var.count('=')) {
                stringList = var.split('=');
                m_macroCoefficients[stringList.first()] = MathParser(m_macroCoefficients).Parse(stringList.last().replace(QChar('x'), '*', Qt::CaseInsensitive));
                continue;
            }
            else {
                for (QString& var2 : var.split(',')) {
                    if (var2.count('$')) {
                        mod.push_back(MathParser(m_macroCoefficients).Parse(var2.replace(QChar('x'), '*', Qt::CaseInsensitive)));
                    }
                    else {
                        mod.push_back(var2.toDouble());
                    }
                }
            }
            if (mod.size() < 2)
                continue;

            m_paths.clear();
            polygon.clear();
            clipper.Clear();

            exposure = mod[1];
            switch ((int)mod[0]) {
            case COMMENT:
                qDebug() << "Macro comment:" << var;
                continue;
            case CIRCLE:
                exposure = mod[1];
                mod[2] *= uScale;
                mod[3] *= uScale;
                mod[4] *= uScale;
                center = IntPoint(mod[3], mod[4]);
                polygon = circle(mod[2], center);
                if (mod.size() > 5) {
                    rotate(polygon, mod[5], center);
                }
                m_paths.push_back(polygon);
                break;
            case OUTLINE_CUSTOM_POLYGON: //exposure
                exposure = mod[1];
                polygon.clear();
                for (int j = 0; j < int(mod[2]); ++j) {
                    polygon.push_back(IntPoint(mod[3 + j * 2] * uScale, mod[4 + j * 2] * uScale));
                }
                if (mod.size() > (mod[2] * 2 + 3) && mod.last() > 0) {
                    rotate(polygon, mod.last());
                }
                m_paths.push_back(polygon);
                break;
            case OUTLINE_REGULAR_POLYGON: //exposure
                exposure = mod[1];
                mod[3] *= uScale;
                mod[4] *= uScale;
                mod[5] *= uScale;
                if (3 > mod[2] || mod[2] > 12) {
                    throw QString("Bad outline (regular polygon) macro!");
                }
                center = IntPoint(mod[3], mod[4]);
                mod[5] /= 2.0;
                for (int j = 0; j < mod[2]; ++j) {
                    polygon.push_back(IntPoint(
                        qCos(qDegreesToRadians(j * 360.0 / mod[2])) * mod[5],
                        qSin(qDegreesToRadians(j * 360.0 / mod[2])) * mod[5]));
                }
                if (mod.size() > 6) {
                    rotate(polygon, mod[6]);
                }
                translate(polygon, center);
                m_paths.push_back(polygon);
                break;
            case MOIRE:
                exposure = 1;
                mod[1] *= uScale;
                mod[2] *= uScale;
                mod[3] *= uScale;
                mod[4] *= uScale;
                mod[5] *= uScale;
                mod[7] *= uScale;
                mod[8] *= uScale;
                center = IntPoint(mod[1], mod[2]);
                for (int num = 0; num < mod[6]; ++num) {
                    clipper.AddPath(circle(mod[3]), ptClip, true);
                    mod[3] -= mod[4] * 2;
                    polygon = circle(mod[3]);
                    ReversePath(polygon);
                    clipper.AddPath(polygon, ptClip, true);
                    mod[3] -= mod[5] * 2;
                }
                clipper.AddPath(rect(mod[8], mod[7]), ptClip, true);
                clipper.AddPath(rect(mod[7], mod[8]), ptClip, true);
                clipper.Execute(ctUnion, m_paths, pftPositive, pftPositive);
                if (mod.size() > 9) {
                    for (Path& path : m_paths) {
                        rotate(path, mod[9], center);
                    }
                }
                return;
            case THERMAL:
                exposure = 1;
                if (mod[3] <= mod[4] || mod[4] < 0.0 || mod[5] >= (mod[3] / qPow(2.0, 0.5))) {
                    throw QString("Bad termal macro!");
                }
                mod[1] *= uScale;
                mod[2] *= uScale;
                mod[3] *= uScale;
                mod[4] *= uScale;
                mod[5] *= uScale;
                center = IntPoint(mod[1], mod[2]);
                clipper.AddPath(circle(mod[3]), ptSubject, true);
                clipper.AddPath(circle(mod[4]), ptClip, true);
                clipper.AddPath(rect(mod[5], mod[3]), ptClip, true);
                clipper.AddPath(rect(mod[3], mod[5]), ptClip, true);
                clipper.Execute(ctDifference, m_paths, pftNonZero, pftNonZero);
                if (mod.size() > 6) {
                    for (Path& path : m_paths) {
                        rotate(path, mod[6], center);
                    }
                }
                return;
            case VECTOR_LINE:
                exposure = mod[1];
                mod[2] *= uScale;
                mod[3] *= uScale;
                mod[4] *= uScale;
                mod[5] *= uScale;
                mod[6] *= uScale;
                center = IntPoint((mod[3] + mod[5]) / 2, (mod[4] + mod[6]) / 2);
                polygon = rect(Length(IntPoint(mod[3], mod[4]), IntPoint(mod[5], mod[6])), mod[2], center);
                rotate(polygon, Angle(IntPoint(mod[3], mod[4]), IntPoint(mod[5], mod[6])), center);
                if (mod.size() > 7) {
                    rotate(polygon, mod[7]);
                }
                m_paths.push_back(polygon);
                break;
            case CENTER_LINE:
                exposure = mod[1];
                mod[2] *= uScale;
                mod[3] *= uScale;
                mod[4] *= uScale;
                mod[5] *= uScale;
                center = IntPoint(mod[4], mod[5]);
                polygon = rect(mod[2], mod[3], center);
                if (mod.size() > 6) {
                    rotate(polygon, mod[6], center);
                }
                m_paths.push_back(polygon);
                break;
            }
            if (Area(m_paths[m_paths.size() - 1]) < 0) {
                if (exposure) {
                    ReversePath(m_paths[m_paths.size() - 1]);
                }
            }
            else {
                if (!exposure) {
                    ReversePath(m_paths[m_paths.size() - 1]);
                }
            }
            items.push_back(qMakePair(exposure, m_paths));
        }
    }
    catch (...) {
        qDebug() << "GAMacro draw error";
        throw;
    }

    if (items.size() > 1) {
        for (int i = 0, exp = -1; i < items.size();) {
            clipper.Clear();
            clipper.AddPaths(m_paths, ptSubject, true);
            exp = items[i].first;
            do {
                clipper.AddPaths((const Paths)items[i++].second, ptClip, true);
            } while (i < items.size() && exp == items[i].first);
            if (exp) {
                clipper.Execute(ctUnion, m_paths, pftNonZero, pftNonZero);
            }
            else {
                clipper.Execute(ctDifference, m_paths, pftNonZero, pftNonZero);
            }
        }
    }

    ClipperBase clipperBase;
    clipperBase.AddPaths(m_paths, ptSubject, true);
    IntRect rect = clipperBase.GetBounds();
    rect.top -= rect.bottom;
    rect.right -= rect.left;
    double x = rect.right * dScale;
    double y = rect.top * dScale;
    m_size = qSqrt(x * x + y * y);
}
