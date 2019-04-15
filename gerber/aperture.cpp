#include "aperture.h"
#include "mathparser.h"
#include <QDebug>
#include <QLineF>

namespace Gerber {

AbstractAperture::AbstractAperture(const Format* format)
    : m_format(format)
{
}

AbstractAperture::~AbstractAperture() {}

double AbstractAperture::drillDiameter() const
{
    return m_drillDiam;
}

Paths AbstractAperture::draw(const State& state)
{
    if (state.dCode() == D03)
        m_isFlashed = true;
    if (m_size == 0)
        draw();

    Paths tmpPpaths(m_paths);

    for (Path& path : tmpPpaths) {
        if (state.imgPolarity() == Negative)
            ReversePath(path);
        if (m_format->unitMode == Inches && type() == Macro)
            for (IntPoint& pt : path) {
                pt.X *= 25.4;
                pt.Y *= 25.4;
            }

        transform(path, state);

        //if (state.curPos().X != 0 || state.curPos().Y != 0)
        TranslatePath(path, state.curPos());
    }
    //    Paths tmpPpaths;
    //    tmpPpaths.reserve(m_paths.size());

    //    for (Path path : m_paths) {
    //        if (state.imgPolarity() == Negative)
    //            ReversePath(path);
    //        if (m_format->unitMode == Inches && type() == Macro)
    //            for (IntPoint& pt : path) {
    //                pt.X *= 25.4;
    //                pt.Y *= 25.4;
    //            }

    //        transform(path, state);

    //        if (state.curPos().X != 0 || state.curPos().Y != 0)
    //            translate(path, state.curPos());
    //        tmpPpaths.push_back(path);
    //    }
    return tmpPpaths;
}

double AbstractAperture::apSize() const
{
    return m_size;
}

double AbstractAperture::apertureSize()
{
    if (m_size == 0)
        draw();
    return m_size;
}

Path AbstractAperture::drawDrill(const State& state)
{
    if (qFuzzyIsNull(m_drillDiam))
        return Path();

    Path drill = CirclePath(m_drillDiam * uScale);

    if (state.imgPolarity() == Positive)
        ReversePath(drill);

    TranslatePath(drill, state.curPos());
    return drill;
}

void AbstractAperture::transform(Path& poligon, const State& state)
{

    bool fl = Area(poligon) < 0;
    for (IntPoint& pt : poligon) {

        if (state.mirroring() & X_Mirroring)
            pt.X = -pt.X;
        if (state.mirroring() & Y_Mirroring)
            pt.Y = -pt.Y;
        if (state.rotating() != 0.0 || state.scaling() != 1.0) {
            const double tmpAangle = qDegreesToRadians(state.rotating() - Angle(IntPoint(), pt));
            const double length = Length(IntPoint(), pt) * state.scaling();
            pt = IntPoint(qCos(tmpAangle) * length, qSin(tmpAangle) * length);
        }
    }

    if (fl != (Area(poligon) < 0))
        ReversePath(poligon);
}

/////////////////////////////////////////////////////
/// \brief ApCircle::ApCircle
/// \param diam
/// \param drillDiam
/// \param format
///
ApCircle::ApCircle(double diam, double drillDiam, const Format* format)
    : AbstractAperture(format)
{
    m_diam = diam;
    m_drillDiam = drillDiam;
    // GerberAperture interface
}

QString ApCircle::name() { return QString("CIRC(Ø%1)").arg(m_diam); } //CIRCLE

ApertureType ApCircle::type() const { return Circle; }

bool ApCircle::fit(double toolDiam) const { return m_diam > toolDiam; }

void ApCircle::draw()
{
    m_paths.push_back(CirclePath(m_diam * uScale));
    m_size = m_diam;
}
/////////////////////////////////////////////////////
/// \brief ApRectangle::ApRectangle
/// \param width
/// \param height
/// \param drillDiam
/// \param format
///
ApRectangle::ApRectangle(double width, double height, double drillDiam, const Format* format)
    : AbstractAperture(format)
{
    m_width = width;
    m_height = height;
    m_drillDiam = drillDiam;
}

QString ApRectangle::name() //RECTANGLE
{
    if (m_width == m_height)
        return QString("RECT(□ %1)").arg(m_width);
    else
        return QString("RECT(%1 x %2)").arg(m_width).arg(m_height);
}

ApertureType ApRectangle::type() const { return Rectangle; }

bool ApRectangle::fit(double toolDiam) const { return qMin(m_height, m_width) > toolDiam; }

void ApRectangle::draw()
{
    m_paths.push_back(RectanglePath(m_width * uScale, m_height * uScale));
    m_size = qSqrt(m_width * m_width + m_height * m_height);
}
/////////////////////////////////////////////////////
/// \brief ApObround::ApObround
/// \param width
/// \param height
/// \param drillDiam
/// \param format
///
ApObround::ApObround(double width, double height, double drillDiam, const Format* format)
    : AbstractAperture(format)
{

    m_width = width;
    m_height = height;
    m_drillDiam = drillDiam;
}

QString ApObround::name() { return QString("OBRO(%1 x %2)").arg(m_width).arg(m_height); } //OBROUND

ApertureType ApObround::type() const { return Obround; }

bool ApObround::fit(double toolDiam) const { return qMin(m_height, m_width) > toolDiam; }

void ApObround::draw()
{
    Clipper clipper;
    cInt height_ = m_height * uScale;
    cInt width_ = m_width * uScale;
    if (qFuzzyCompare(width_ + 1.0, height_ + 1.0)) {
        m_paths.push_back(CirclePath(width_));
    } else {
        if (width_ > height_) {
            clipper.AddPath(CirclePath(height_, IntPoint(-(width_ - height_) / 2, 0)), ptClip, true);
            clipper.AddPath(CirclePath(height_, IntPoint((width_ - height_) / 2, 0)), ptClip, true);
            clipper.AddPath(RectanglePath(width_ - height_, height_), ptClip, true);
        } else if (width_ < height_) {
            clipper.AddPath(CirclePath(width_, IntPoint(0, -(height_ - width_) / 2)), ptClip, true);
            clipper.AddPath(CirclePath(width_, IntPoint(0, (height_ - width_) / 2)), ptClip, true);
            clipper.AddPath(RectanglePath(width_, height_ - width_), ptClip, true);
        }
        clipper.Execute(ctUnion, m_paths, pftNonZero, pftNonZero);
    }
    m_size = qMax(width_, height_);
}
/////////////////////////////////////////////////////
/// \brief ApPolygon::ApPolygon
/// \param diam
/// \param nVertices
/// \param rotation
/// \param drillDiam
/// \param format
///
ApPolygon::ApPolygon(double diam, int nVertices, double rotation, double drillDiam, const Format* format)
    : AbstractAperture(format)
{
    m_diam = diam;
    m_verticesCount = nVertices;
    m_rotation = rotation;
    m_drillDiam = drillDiam;
}

double ApPolygon::rotation() const { return m_rotation; }

int ApPolygon::verticesCount() const { return m_verticesCount; }

QString ApPolygon::name() { return QString("POLY(Ø%1, N%2)").arg(m_diam).arg(m_verticesCount); } //POLYGON

ApertureType ApPolygon::type() const { return Polygon; }

bool ApPolygon::fit(double toolDiam) const { return m_diam * cos(M_PI / m_verticesCount) > toolDiam; }

void ApPolygon::draw()
{
    Path poligon;
    const double step = 360.0 / m_verticesCount;
    const double diam = this->m_diam * uScale;
    for (int i = 0; i < m_verticesCount; ++i) {
        poligon.push_back(IntPoint(qCos(qDegreesToRadians(step * i)) * diam / 2.0, qSin(qDegreesToRadians(step * i)) * diam / 2.0));
    }
    if (m_rotation > 0.1) {
        RotatePath(poligon, m_rotation);
    }
    m_paths.push_back(poligon);
    m_size = diam;
}
/////////////////////////////////////////////////////
/// \brief ApMacro::ApMacro
/// \param macro
/// \param modifiers
/// \param coefficients
/// \param format
///
ApMacro::ApMacro(const QString& macro, const QList<QString>& modifiers, const QMap<QString, double>& coefficients, const Format* format)
    : AbstractAperture(format)
{
    m_macro = macro;
    m_modifiers = modifiers;
    while (m_modifiers.last().isEmpty()) {
        m_modifiers.removeLast();
    }
    m_coefficients = coefficients;
}

QString ApMacro::name() { return QString("MACRO(%1)").arg(m_macro); } //MACRO

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

    QMap<QString, double> macroCoefficients{ m_coefficients };

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
    Path polygon = RectanglePath(mod[Width] * uScale, mod[Height] * uScale, center);

    if (mod.size() > RotationAngle && mod[RotationAngle] != 0.0)
        RotatePath(polygon, mod[RotationAngle]);

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

    Path polygon = CirclePath(mod[Diameter] * uScale, center);

    if (mod.size() > RotationAngle && mod[RotationAngle] != 0.0)
        RotatePath(polygon, mod[RotationAngle]);

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
        clipper.AddPath(CirclePath(diameter), ptClip, true);
        diameter -= thickness * 2;
        Path polygon = CirclePath(diameter);
        ReversePath(polygon);
        clipper.AddPath(polygon, ptClip, true);
        diameter -= gap * 2;
    }
    clipper.AddPath(RectanglePath(cl, ct), ptClip, true);
    clipper.AddPath(RectanglePath(ct, cl), ptClip, true);
    clipper.Execute(ctUnion, m_paths, pftPositive, pftPositive);

    for (Path& path : m_paths)
        TranslatePath(path, center);

    if (mod.size() > RotationAngle && mod[RotationAngle] != 0.0) {
        for (Path& path : m_paths)
            RotatePath(path, mod[RotationAngle]);
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
        RotatePath(polygon, mod.last());

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
        RotatePath(polygon, mod[RotationAngle]);

    TranslatePath(polygon, center);

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
    clipper.AddPath(CirclePath(outer), ptSubject, true);
    clipper.AddPath(CirclePath(inner), ptClip, true);
    clipper.AddPath(RectanglePath(gap, outer), ptClip, true);
    clipper.AddPath(RectanglePath(outer, gap), ptClip, true);
    clipper.Execute(ctDifference, m_paths, pftNonZero, pftNonZero);

    for (Path& path : m_paths)
        TranslatePath(path, center);

    if (mod.size() > RotationAngle && mod[RotationAngle] != 0.0) {
        for (Path& path : m_paths)
            RotatePath(path, mod[RotationAngle]);
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

    Path polygon = RectanglePath(Length(start, end), mod[Width] * uScale);
    double angle = Angle(start, end);
    RotatePath(polygon, angle);
    TranslatePath(polygon, center);

    if (mod.size() > RotationAngle && mod[RotationAngle] != 0.0)
        RotatePath(polygon, mod[RotationAngle]);

    return polygon;
}
/////////////////////////////////////////////////////
/// \brief ApBlock::ApBlock
/// \param macro
/// \param modifiers
/// \param coefficients
/// \param format
///
ApBlock::ApBlock(const Format* format)
    : AbstractAperture(format)
{
}

QString ApBlock::name() { return QString("BLOCK"); }

ApertureType ApBlock::type() const { return Block; }

void ApBlock::draw()
{
    m_paths.clear();
    int i = 0;
    while (i < size()) {
        Clipper clipper; //(ioStrictlySimple);
        clipper.AddPaths(m_paths, ptSubject, true);
        const int exp = at(i).state.imgPolarity();
        do {
            m_paths.append(at(i).paths);
            clipper.AddPaths(at(i++).paths, ptClip, true);
        } while (i < size() && exp == at(i).state.imgPolarity());
        if (at(i - 1).state.imgPolarity() == Positive)
            clipper.Execute(ctUnion, m_paths, pftPositive);
        else
            clipper.Execute(ctDifference, m_paths, pftNonZero);
    }
    m_size = 1;
    qDebug() << m_paths.size();
    //CleanPolygons(m_paths, 0.0009 * uScale);
}

} // namespace Gerber
