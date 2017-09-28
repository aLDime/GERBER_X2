#include "gerberaperture.h"
#include "mathparser.h"

#include "gerber.h"
#include <QtMath>
#include "../clipper/myclipper.h"

GerberAperture::GerberAperture()
    : diam(0)
    , height(0)
    , holeDiam(0)
    , numVertices(0)
    , rotation(0)
    , size(0)
    , width(0)
{
}

Paths GerberAperture::Draw(const STATE& state)
{
    if (!paths.size()) {
        Draw();
    }

    Paths tmpPpaths;
    for (Path var : paths) {
        if (state.imgPolarity == NEGATIVE) {
            ReversePath(var);
        }
        if (state.format.unitMode == INCHES && type == APERTURE_MACRO) {
            for (IntPoint& pt : var) {
                pt.X *= 25.4;
                pt.Y *= 25.4;
            }
        }
        if (state.curPos.X != 0 || state.curPos.Y != 0) {
            Translate(var, state.curPos);
        }
        tmpPpaths.push_back(var);
    }

    //    Path var = hole;
    //    if (state.imgPolarity == NEGATIVE) {
    //        ReversePath(var);
    //    }
    //    if (state.format.unitMode == INCHES && type == APERTURE_MACRO) {
    //        for (int i = 0; i < var.size(); ++i) {
    //            var[i].X *= 25.4;
    //            var[i].Y *= 25.4;
    //        }
    //    }
    //    if (state.curPos.X != 0 && state.curPos.Y != 0) {
    //        Translate(var, state.curPos);
    //    }
    //    tmpPpaths.push_back(var);

    return tmpPpaths;
}

double GerberAperture::Size()
{
    if (size == 0)
        Draw();
    return size;
}

void GerberAperture::SetDiameter(double diam_) { diam = diam_; }

void GerberAperture::SetHoleDiameter(double holeDiam_) { holeDiam = holeDiam_; }

void GerberAperture::SetWidth(double width_) { width = width_; }

void GerberAperture::SetHeight(double height_) { height = height_; }

void GerberAperture::SetMacro(const QString& macro_) { macro = macro_; }

void GerberAperture::SetMacroCoefficients(const QMap<QString, double>& macroCoefficients_) { macroCoefficients = macroCoefficients_; }

void GerberAperture::SetNumVertices(int nVertices_) { numVertices = nVertices_; }

void GerberAperture::SetRotation(double rotation_) { rotation = rotation_; }

void GerberAperture::SetSize(double size_) { size = size_; }

void GerberAperture::SetType(APERTURE_TYPE type_) { type = type_; }

void GerberAperture::SetModifiers(const QList<QString>& modifiers_)
{
    modifiers = modifiers_;
    while (modifiers.last().isEmpty()) {
        modifiers.removeLast();
    }
}

Path GerberAperture::GetHole(const STATE& state)
{
    //    if (holeDiam > 0)
    //        qDebug() << "";
    //    if (qFuzzyIsNull(holeDiam))
    //        return hole;
    //    if (hole.empty()) {
    //        hole = Circle(holeDiam * uScale);
    //        ReversePath(hole);
    //    }
    //    if (state.curPos.X != 0 || state.curPos.Y != 0) {
    //        Path var = hole;
    //        Translate(var, state.curPos);
    //        return var;
    //    }
    hole = Circle(holeDiam * uScale);
    if (state.imgPolarity == POSITIVE) {
        ReversePath(hole);
    }
    Translate(hole, state.curPos);
    return hole;
}

void GerberAperture::Draw()
{
    switch (type) {
    case CIRCULAR:
        DrawCircular();
        break;
    case RECTANGLE:
        DrawRectangle();
        break;
    case OBROUND:
        DrawObround();
        break;
    case POLYGON:
        DrawPolygon();
        break;
    case APERTURE_MACRO:
        DrawMacro();
        break;
    }
}

Path GerberAperture::Circle(double diametr, IntPoint center)
{
    Path poligon(STEPS_PER_CIRCLE);
    double radius = diametr / 2.0;
    for (int i = 0; i < STEPS_PER_CIRCLE; ++i) {
        poligon[i] = IntPoint(
            (qCos(qDegreesToRadians((double)i * 360.0 / STEPS_PER_CIRCLE)) * radius) + center.X,
            (qSin(qDegreesToRadians((double)i * 360.0 / STEPS_PER_CIRCLE)) * radius) + center.Y);
    }
    //    poligon.push_back(poligon[0]);
    if (Area(poligon) < 0) {
        ReversePath(poligon);
    }
    return poligon;
}

Path GerberAperture::Rect(double width, double height, IntPoint center)
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

Path& GerberAperture::Rotate(Path& poligon, double angle, IntPoint center)
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

void GerberAperture::Translate(Path& path, IntPoint pos)
{
    if (pos.X == 0 && pos.Y == 0)
        return;
    for (Path::size_type i = 0, size = path.size(); i < size; ++i) {
        path[i].X += pos.X;
        path[i].Y += pos.Y;
    }
}

void GerberAperture::DrawCircular()
{
    paths.push_back(Circle(diam * uScale));
    size = diam;
}

void GerberAperture::DrawMacro()
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

    for (int i = 0; i < modifiers.size(); ++i) {
        QString var = modifiers[i];
        mod.clear();
        if (var.at(0) == '0') {
            qDebug() << "Macro comment:" << var;
            continue;
        }
        if (var.count('=')) {
            stringList = var.split('=');
            macroCoefficients[stringList.first()] = MathParser(macroCoefficients).Parse(stringList.last().replace(QChar('x'), '*', Qt::CaseInsensitive));
            continue;
        }
        else {
            for (QString& var2 : var.split(',')) {
                if (var2.count('$')) {
                    mod.push_back(MathParser(macroCoefficients).Parse(var2.replace(QChar('x'), '*', Qt::CaseInsensitive)));
                }
                else {
                    mod.push_back(var2.toDouble());
                }
            }
        }
        if (mod.size() < 2)
            continue;

        paths.clear();
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
            polygon = Circle(mod[2], center);
            if (mod.size() > 5) {
                Rotate(polygon, mod[5], center);
            }
            paths.push_back(polygon);
            break;
        case OUTLINE_CUSTOM_POLYGON: //exposure
            exposure = mod[1];
            polygon.clear();
            for (int j = 0; j <= int(mod[2]); ++j) {
                polygon.push_back(IntPoint(mod[j * 2 + 3] * uScale, mod[j * 2 + 4] * uScale));
            }
            if (mod.size() > (int(mod[2]) * 2 + 5) && mod.last() > 0) {
                Rotate(polygon, mod.last());
            }
            paths.push_back(polygon);
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
                Rotate(polygon, mod[6]);
            }
            Translate(polygon, center);
            paths.push_back(polygon);
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
                clipper.AddPath(Circle(mod[3]), ptClip, true);
                mod[3] -= mod[4] * 2;
                polygon = Circle(mod[3]);
                ReversePath(polygon);
                clipper.AddPath(polygon, ptClip, true);
                mod[3] -= mod[5] * 2;
            }
            clipper.AddPath(Rect(mod[8], mod[7]), ptClip, true);
            clipper.AddPath(Rect(mod[7], mod[8]), ptClip, true);
            clipper.Execute(ctUnion, paths, pftPositive, pftPositive);
            if (mod.size() > 9) {
                for (Path& path : paths) {
                    Rotate(path, mod[9], center);
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
            clipper.AddPath(Circle(mod[3]), ptSubject, true);
            clipper.AddPath(Circle(mod[4]), ptClip, true);
            clipper.AddPath(Rect(mod[5], mod[3]), ptClip, true);
            clipper.AddPath(Rect(mod[3], mod[5]), ptClip, true);
            clipper.Execute(ctDifference, paths, pftNonZero, pftNonZero);
            if (mod.size() > 6) {
                for (Path& path : paths) {
                    Rotate(path, mod[6], center);
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
            polygon = Rect(Length(IntPoint(mod[3], mod[4]), IntPoint(mod[5], mod[6])), mod[2], center);
            Rotate(polygon, Angle(IntPoint(mod[3], mod[4]), IntPoint(mod[5], mod[6])), center);
            if (mod.size() > 7) {
                Rotate(polygon, mod[7]);
            }
            paths.push_back(polygon);
            break;
        case CENTER_LINE:
            exposure = mod[1];
            mod[2] *= uScale;
            mod[3] *= uScale;
            mod[4] *= uScale;
            mod[5] *= uScale;
            center = IntPoint(mod[4], mod[5]);
            polygon = Rect(mod[2], mod[3], center);
            if (mod.size() > 6) {
                Rotate(polygon, mod[6], center);
            }
            paths.push_back(polygon);
            break;
        }
        if (Area(paths[paths.size() - 1]) < 0) {
            if (exposure) {
                ReversePath(paths[paths.size() - 1]);
            }
        }
        else {
            if (!exposure) {
                ReversePath(paths[paths.size() - 1]);
            }
        }
        items.push_back(qMakePair(exposure, paths));
    }

    if (items.size() > 1) {
        for (int i = 0, exp = -1; i < items.size();) {
            clipper.Clear();
            clipper.AddPaths(paths, ptSubject, true);
            exp = items[i].first;
            do {
                clipper.AddPaths((const Paths)items[i++].second, ptClip, true);
            } while (i < items.size() && exp == items[i].first);
            if (exp) {
                clipper.Execute(ctUnion, paths, pftNonZero, pftNonZero);
            }
            else {
                clipper.Execute(ctDifference, paths, pftNonZero, pftNonZero);
            }
        }
    }

    ClipperBase clipperBase;
    clipperBase.AddPaths(paths, ptSubject, true);
    IntRect rect = clipperBase.GetBounds();
    rect.top -= rect.bottom;
    rect.right -= rect.left;
    double x = rect.right * dScale;
    double y = rect.top * dScale;
    size = qSqrt(x * x + y * y);
}

void GerberAperture::DrawObround()
{
    Clipper clipper;
    cInt height_ = height * uScale;
    cInt width_ = width * uScale;
    if (qFuzzyCompare(width_ + 1.0, height_ + 1.0)) {
        paths.push_back(Circle(width_));
    }
    else {
        if (width_ > height_) {
            clipper.AddPath(Circle(height_, IntPoint(-(width_ - height_) / 2, 0)), ptClip, true);
            clipper.AddPath(Circle(height_, IntPoint((width_ - height_) / 2, 0)), ptClip, true);
            clipper.AddPath(Rect(width_ - height_, height_), ptClip, true);
        }
        else if (width_ < height_) {
            clipper.AddPath(Circle(width_, IntPoint(0, -(height_ - width_) / 2)), ptClip, true);
            clipper.AddPath(Circle(width_, IntPoint(0, (height_ - width_) / 2)), ptClip, true);
            clipper.AddPath(Rect(width_, height_ - width_), ptClip, true);
        }
        clipper.Execute(ctUnion, paths, pftNonZero, pftNonZero);
    }
    size = qMax(width_, height_);
}

void GerberAperture::DrawPolygon()
{
    Path poligon;
    const double step = 360.0 / numVertices;
    const double diam = this->diam * uScale;
    for (int i = 0; i < numVertices; ++i) {
        poligon.push_back(IntPoint(qCos(qDegreesToRadians(step * i)) * diam / 2, qSin(qDegreesToRadians(step * i)) * diam / 2));
    }
    if (rotation > 0.1) {
        poligon = Rotate(poligon, rotation);
    }
    paths.push_back(poligon);
    size = diam;
}

void GerberAperture::DrawRectangle()
{
    paths.push_back(Rect(width * uScale, height * uScale));
    size = qSqrt(width * width + height * height);
}
