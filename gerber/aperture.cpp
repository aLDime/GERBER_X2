#include "aperture.h"
#include "mathparser.h"
#include <QDebug>
#include <QLineF>

using namespace G;

AbstractAperture::AbstractAperture() {}

AbstractAperture::~AbstractAperture() {}

Paths AbstractAperture::draw(const State& state)
{
    if (state.dCode == D03)
        m_isFlashed = true;
    if (m_size == 0)
        draw();

    Paths tmpPpaths;
    for (Path var : m_paths) {
#ifdef DEPRECATED_IMAGE_POLARITY
        if (state.imgPolarity == Negative)
            ReversePath(var);
#endif
        if (state.format->unitMode == Inches && type() == Macro) {
            for (IntPoint& pt : var) {
                pt.X *= 25.4;
                pt.Y *= 25.4;
            }
        }
        if (state.curPos.X != 0 || state.curPos.Y != 0)
            translate(var, state.curPos);
        tmpPpaths.push_back(var);
    }
    return tmpPpaths;
}

double AbstractAperture::size()
{
    if (m_size == 0)
        draw();
    return m_size;
}

Path AbstractAperture::drawDrill(const State& state)
{
    if (qFuzzyIsNull(m_drillDiam))
        return Path();

    Path drill = circle(m_drillDiam * uScale);

    if (state.imgPolarity == Positive)
        ReversePath(drill);

    translate(drill, state.curPos);
    return drill;
}

Path AbstractAperture::circle(double diametr, IntPoint center)
{
    double radius = diametr / 2.0;
    Path poligon(StepsPerCircle);
    for (int i = 0; i < StepsPerCircle; ++i) {
        poligon[i] = IntPoint(
            (qCos(qDegreesToRadians((double)i * 360.0 / StepsPerCircle)) * radius) + center.X,
            (qSin(qDegreesToRadians((double)i * 360.0 / StepsPerCircle)) * radius) + center.Y);
    }

    if (Area(poligon) < 0)
        ReversePath(poligon);

    return poligon;
}

Path AbstractAperture::rectangle(double width, double height, IntPoint center)
{

    const double halfWidth = width * 0.5;
    const double halfHeight = height * 0.5;
    Path poligon{
        IntPoint(-halfWidth + center.X, +halfHeight + center.Y),
        IntPoint(-halfWidth + center.X, -halfHeight + center.Y),
        IntPoint(+halfWidth + center.X, -halfHeight + center.Y),
        IntPoint(+halfWidth + center.X, +halfHeight + center.Y),
    };
    if (Area(poligon) < 0.0)
        ReversePath(poligon);

    return poligon;
}

void AbstractAperture::rotate(Path& poligon, double angle, IntPoint center)
{
    bool fl = Area(poligon) < 0;
    for (IntPoint& pt : poligon) {
        const double tmpAangle = angle - Angle(center, pt);
        const double length = Length(center, pt);
        pt = IntPoint(
            qCos(qDegreesToRadians(tmpAangle)) * length,
            qSin(qDegreesToRadians(tmpAangle)) * length);
    }
    if (fl != (Area(poligon) < 0))
        ReversePath(poligon);
}

void AbstractAperture::translate(Path& path, IntPoint pos)
{
    if (pos.X == 0 && pos.Y == 0)
        return;
    for (Path::size_type i = 0, size = path.size(); i < size; ++i) {
        path[i].X += pos.X;
        path[i].Y += pos.Y;
    }
}
/////////////////////////////////////////////////////

ApCircle::ApCircle(double diam, double drillDiam)
{
    m_diam = diam;
    m_drillDiam = drillDiam;
    // GerberAperture interface
}

QString ApCircle::name() { return QString("CIRCLE(Ø%1)").arg(m_diam); }

ApertureType ApCircle::type() const { return Circle; }

void ApCircle::draw()
{
    m_paths.push_back(circle(m_diam * uScale));
    m_size = m_diam;
}
/////////////////////////////////////////////////////

ApRectangle::ApRectangle(double width, double height, double drillDiam)
{
    m_width = width;
    m_height = height;
    m_drillDiam = drillDiam;
}

QString ApRectangle::name() { return QString("RECTANGLE(W%1, H%2)").arg(m_width).arg(m_height); }

ApertureType ApRectangle::type() const { return Rectangle; }

void ApRectangle::draw()
{
    m_paths.push_back(rectangle(m_width * uScale, m_height * uScale));
    m_size = qSqrt(m_width * m_width + m_height * m_height);
}
/////////////////////////////////////////////////////

ApObround::ApObround(double width, double height, double drillDiam)
{

    m_width = width;
    m_height = height;
    m_drillDiam = drillDiam;
}

QString ApObround::name() { return QString("OBROUND(W%1, H%2)").arg(m_width).arg(m_height); }

ApertureType ApObround::type() const { return Obround; }

void ApObround::draw()
{
    Clipper clipper;
    cInt height_ = m_height * uScale;
    cInt width_ = m_width * uScale;
    if (qFuzzyCompare(width_ + 1.0, height_ + 1.0)) {
        m_paths.push_back(circle(width_));
    } else {
        if (width_ > height_) {
            clipper.AddPath(circle(height_, IntPoint(-(width_ - height_) / 2, 0)), ptClip, true);
            clipper.AddPath(circle(height_, IntPoint((width_ - height_) / 2, 0)), ptClip, true);
            clipper.AddPath(rectangle(width_ - height_, height_), ptClip, true);
        } else if (width_ < height_) {
            clipper.AddPath(circle(width_, IntPoint(0, -(height_ - width_) / 2)), ptClip, true);
            clipper.AddPath(circle(width_, IntPoint(0, (height_ - width_) / 2)), ptClip, true);
            clipper.AddPath(rectangle(width_, height_ - width_), ptClip, true);
        }
        clipper.Execute(ctUnion, m_paths, pftNonZero, pftNonZero);
    }
    m_size = qMax(width_, height_);
}
/////////////////////////////////////////////////////

ApPolygon::ApPolygon(double diam, int nVertices, double rotation, double drillDiam)
{
    m_diam = diam;
    m_verticesCount = nVertices;
    m_rotation = rotation;
    m_drillDiam = drillDiam;
}

double ApPolygon::rotation() const { return m_rotation; }

int ApPolygon::verticesCount() const { return m_verticesCount; }

QString ApPolygon::name() { return QString("POLYGON(Ø%1, N%2)").arg(m_diam).arg(m_verticesCount); }

ApertureType ApPolygon::type() const { return Polygon; }

void ApPolygon::draw()
{
    Path poligon;
    const double step = 360.0 / m_verticesCount;
    const double diam = this->m_diam * uScale;
    for (int i = 0; i < m_verticesCount; ++i) {
        poligon.push_back(IntPoint(qCos(qDegreesToRadians(step * i)) * diam / 2, qSin(qDegreesToRadians(step * i)) * diam / 2));
    }
    if (m_rotation > 0.1) {
        rotate(poligon, m_rotation);
    }
    m_paths.push_back(poligon);
    m_size = diam;
}
/////////////////////////////////////////////////////

ApMacro::ApMacro(const QString& macro, const QList<QString>& modifiers, const QMap<QString, double>& macroCoefficients)
{
    m_macro = macro;
    m_modifiers = modifiers;
    while (m_modifiers.last().isEmpty()) {
        m_modifiers.removeLast();
    }
    m_macroCoefficients = macroCoefficients;
}

QString ApMacro::name() { return QString("MACRO(%1)").arg(m_macro); }

ApertureType ApMacro::type() const { return Macro; }

void ApMacro::draw()
{
    enum {
        Comment = 0,
        Circle = 1,
        OutlineCustomPolygon = 4, // MAXIMUM 5000 POINTS
        OutlineRegularPolygon = 5, // 3-12 POINTS
        Moire = 6,
        Thermal = 7,
        VectorLine = 20,
        CenterLine = 21,
    };

    QList<double> mod;
    Path polygon;

    QMap<QString, double> macroCoefficients{ m_macroCoefficients };

    QVector<QPair<bool, Path>> items;
    try {
        for (int i = 0; i < m_modifiers.size(); ++i) {
            QString var = m_modifiers[i];

            mod.clear();

            if (var.at(0) == '0') {
                //qDebug() << "Macro comment:" << var;
                continue;
            }

            if (var.contains('=')) {
                QList<QString> stringList = var.split('=');
                macroCoefficients[stringList.first()]
                    = MathParser(macroCoefficients).parse(stringList.last().replace(QChar('x'), '*', Qt::CaseInsensitive));
                continue;
            } else {
                for (QString& var2 : var.split(',')) {
                    if (var2.contains('$')) {
                        mod.push_back(MathParser(macroCoefficients).parse(var2.replace(QChar('x'), '*', Qt::CaseInsensitive)));
                        //qDebug() << "MathParser" << var2 << "=" << mod.last();
                    } else {
                        mod.push_back(var2.toDouble());
                    }
                }
            }

            //qDebug() << macroCoefficients;

            if (mod.size() < 2)
                continue;

            const bool exposure = mod[1];

            switch ((int)mod[0]) {
            case Comment:
                //qDebug() << "Macro comment2:" << var;
                continue;
            case Circle:
                polygon = drawCircle(mod);
                break;
            case OutlineCustomPolygon:
                polygon = drawOutlineCustomPolygon(mod);
                break;
            case OutlineRegularPolygon:
                polygon = drawOutlineRegularPolygon(mod);
                break;
            case Moire:
                drawMoire(mod);
                return;
            case Thermal:
                drawThermal(mod);
                return;
            case VectorLine:
                polygon = drawVectorLine(mod);
                break;
            case CenterLine:
                polygon = drawCenterLine(mod);
                break;
            }
            if (Area(polygon) < 0) {
                if (exposure)
                    ReversePath(polygon);
            } else {
                if (!exposure)
                    ReversePath(polygon);
            }
            items.push_back({ exposure, polygon });
        }
    } catch (...) {
        qWarning() << "GAMacro draw error";
        throw;
    }

    if (items.size() > 1) {
        Clipper clipper;
        for (int i = 0; i < items.size();) {
            clipper.Clear();
            clipper.AddPaths(m_paths, ptSubject, true);
            bool exp = items[i].first;
            while (i < items.size() && exp == items[i].first)
                clipper.AddPath(items[i++].second, ptClip, true);
            if (exp)
                clipper.Execute(ctUnion, m_paths, pftNonZero, pftNonZero);
            else
                clipper.Execute(ctDifference, m_paths, pftNonZero, pftNonZero);
        }
    } else
        m_paths.append(polygon);

    ClipperBase clipperBase;
    clipperBase.AddPaths(m_paths, ptSubject, true);
    IntRect rect = clipperBase.GetBounds();
    rect.top -= rect.bottom;
    rect.right -= rect.left;
    double x = rect.right * dScale;
    double y = rect.top * dScale;
    m_size = qSqrt(x * x + y * y);
}

Path ApMacro::drawCenterLine(const QList<double>& mod)
{
    enum {
        Width = 2,
        Height,
        CenterX,
        CenterY,
        RotationAngle
    };

    IntPoint center(mod[CenterX] * uScale, mod[CenterY] * uScale);
    Path polygon = rectangle(mod[Width] * uScale, mod[Height] * uScale, center);

    if (mod.size() > RotationAngle && mod[RotationAngle] != 0.0)
        rotate(polygon, mod[RotationAngle]);

    return polygon;
}

Path ApMacro::drawCircle(const QList<double>& mod)
{
    enum {
        Diameter = 2,
        CenterX,
        CenterY,
        RotationAngle
    };

    IntPoint center(mod[CenterX] * uScale, mod[CenterY] * uScale);

    Path polygon = circle(mod[Diameter] * uScale, center);

    if (mod.size() > RotationAngle && mod[RotationAngle] != 0.0)
        rotate(polygon, mod[RotationAngle]);

    return polygon;
}

void ApMacro::drawMoire(const QList<double>& mod)
{
    enum {
        CenterX = 1,
        CenterY,
        Diameter,
        Thickness,
        Gap,
        NumberOfRings,
        CrossThickness,
        CrossLength,
        RotationAngle,
    };

    cInt diameter = mod[Diameter] * uScale;
    cInt thickness = mod[Thickness] * uScale;
    cInt gap = mod[Gap] * uScale;
    cInt ct = mod[CrossThickness] * uScale;
    cInt cl = mod[CrossLength] * uScale;

    Clipper clipper;

    IntPoint center(mod[CenterX] * uScale, mod[CenterY] * uScale);
    for (int num = 0; num < mod[NumberOfRings]; ++num) {
        clipper.AddPath(circle(diameter), ptClip, true);
        diameter -= thickness * 2;
        Path polygon = circle(diameter);
        ReversePath(polygon);
        clipper.AddPath(polygon, ptClip, true);
        diameter -= gap * 2;
    }
    clipper.AddPath(rectangle(cl, ct), ptClip, true);
    clipper.AddPath(rectangle(ct, cl), ptClip, true);
    clipper.Execute(ctUnion, m_paths, pftPositive, pftPositive);

    for (Path& path : m_paths)
        translate(path, center);

    if (mod.size() > RotationAngle && mod[RotationAngle] != 0.0) {
        for (Path& path : m_paths)
            rotate(path, mod[RotationAngle]);
    }
}

Path ApMacro::drawOutlineCustomPolygon(const QList<double>& mod)
{
    enum {
        NumberOfVertices = 2,
        X,
        Y,
    };

    Path polygon;
    int num = mod[NumberOfVertices];
    for (int j = 0; j < int(num); ++j)
        polygon.push_back(IntPoint(mod[X + j * 2] * uScale, mod[Y + j * 2] * uScale));

    if (mod.size() > (num * 2 + 3) && mod.last() > 0)
        rotate(polygon, mod.last());

    return polygon;
}

Path ApMacro::drawOutlineRegularPolygon(const QList<double>& mod)
{
    enum {
        NumberOfVertices = 2,
        CenterX,
        CenterY,
        Diameter,
        RotationAngle
    };
    int num = mod[NumberOfVertices];
    if (3 > num || num > 12)
        throw QString("Bad outline (regular polygon) macro!");

    cInt diameter = mod[Diameter] * uScale * 0.5;
    IntPoint center(mod[CenterX] * uScale, mod[CenterY] * uScale);

    Path polygon;
    for (int j = 0; j < num; ++j)
        polygon.push_back(IntPoint(
            qCos(qDegreesToRadians(j * 360.0 / num)) * diameter,
            qSin(qDegreesToRadians(j * 360.0 / num)) * diameter));

    if (mod.size() > RotationAngle && mod[RotationAngle] != 0.0)
        rotate(polygon, mod[RotationAngle]);

    translate(polygon, center);

    return polygon;
}

void ApMacro::drawThermal(const QList<double>& mod)
{
    enum {
        CenterX = 1,
        CenterY,
        OuterDiameter,
        InnerDiameter,
        GapThickness,
        RotationAngle
    };

    if (mod[OuterDiameter] <= mod[InnerDiameter] || mod[InnerDiameter] < 0.0 || mod[GapThickness] >= (mod[OuterDiameter] / qPow(2.0, 0.5)))
        throw QString("Bad termal macro!");

    cInt outer = mod[OuterDiameter] * uScale;
    cInt inner = mod[InnerDiameter] * uScale;
    cInt gap = mod[GapThickness] * uScale;

    IntPoint center(mod[CenterX] * uScale, mod[CenterY] * uScale);
    Clipper clipper;
    clipper.AddPath(circle(outer), ptSubject, true);
    clipper.AddPath(circle(inner), ptClip, true);
    clipper.AddPath(rectangle(gap, outer), ptClip, true);
    clipper.AddPath(rectangle(outer, gap), ptClip, true);
    clipper.Execute(ctDifference, m_paths, pftNonZero, pftNonZero);

    for (Path& path : m_paths)
        translate(path, center);

    if (mod.size() > RotationAngle && mod[RotationAngle] != 0.0) {
        for (Path& path : m_paths)
            rotate(path, mod[RotationAngle]);
    }
}

Path ApMacro::drawVectorLine(const QList<double>& mod)
{
    enum {
        Width = 2,
        StartX,
        StartY,
        EndX,
        EndY,
        RotationAngle,
    };

    IntPoint start(mod[StartX] * uScale, mod[StartY] * uScale);
    IntPoint end(mod[EndX] * uScale, mod[EndY] * uScale);
    IntPoint center(0.5 * start.X + 0.5 * end.X, 0.5 * start.Y + 0.5 * end.Y);

    Path polygon = rectangle(Length(start, end), mod[Width] * uScale);
    double angle = Angle(start, end);
    rotate(polygon, angle);
    translate(polygon, center);

    if (mod.size() > RotationAngle && mod[RotationAngle] != 0.0)
        rotate(polygon, mod[RotationAngle]);

    return polygon;
}
