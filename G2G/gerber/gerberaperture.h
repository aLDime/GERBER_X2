#ifndef GERBERAPERTURE_H
#define GERBERAPERTURE_H

#include "gerber.h"
#include "mathparser.h"
#include <QtMath>
#include <QMap>

enum APERTURE_TYPE {
    CIRCULAR,
    RECTANGLE,
    OBROUND,
    POLYGON,
    APERTURE_MACRO,
};

class GerberAperture {
public:
    GerberAperture();
    virtual ~GerberAperture();

    bool isDrilled() const { return m_drillDiam != 0.0; }
    bool isFlashed() const { return m_isFlashed; }

    double drillDiameter() const { return m_drillDiam; }
    double size();

    Path drawDrill(const STATE& state);
    Paths draw(const STATE& state);

    virtual QString name() = 0;
    virtual APERTURE_TYPE type() const = 0;

protected:
    bool m_isFlashed = false;
    double m_drillDiam = 0.0;
    double m_size = 0.0;

    Paths m_paths;
    virtual void draw() = 0;

    Path circle(double diametr, IntPoint center = IntPoint());
    Path rect(double m_width, double m_height, IntPoint center = IntPoint());
    Path& rotate(Path& poligon, double angle, IntPoint center = IntPoint());
    void translate(Path& path, IntPoint pos);
};
/////////////////////////////////////////////////////
/// \brief The GACircular class
///
class GACircular : public GerberAperture {
public:
    GACircular(double diam, double drillDiam)
    {
        m_diam = diam;
        m_drillDiam = drillDiam;
        // GerberAperture interface
    }
    QString name() override { return QString("CIRCULAR(D%1)").arg(m_diam); }
    APERTURE_TYPE type() const override { return CIRCULAR; }
protected:
    void draw() override
    {
        m_paths.push_back(circle(m_diam * uScale));
        m_size = m_diam;
    }

private:
    double m_diam = 0.0;
};
/////////////////////////////////////////////////////
/// \brief The GARectangle class
///
class GARectangle : public GerberAperture {
public:
    GARectangle(double width, double height, double drillDiam)
    {
        m_width = width;
        m_height = height;
        m_drillDiam = drillDiam;
    }
    // GerberAperture interface
    QString name() override { return QString("RECTANGLE(W%1, H%2)").arg(m_width).arg(m_height); }
    APERTURE_TYPE type() const override { return RECTANGLE; }
protected:
    void draw() override
    {
        m_paths.push_back(rect(m_width * uScale, m_height * uScale));
        m_size = qSqrt(m_width * m_width + m_height * m_height);
    }

private:
    double m_height = 0.0;
    double m_width = 0.0;
};
/////////////////////////////////////////////////////
/// \brief The GAObround class
///
class GAObround : public GerberAperture {
public:
    GAObround(double width, double height, double drillDiam)
    {

        m_width = width;
        m_height = height;
        m_drillDiam = drillDiam;
    }
    // GerberAperture interface
    QString name() override { return QString("OBROUND(W%1, H%2)").arg(m_width).arg(m_height); }
    APERTURE_TYPE type() const override { return OBROUND; }
protected:
    void draw() override
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

private:
    double m_height = 0.0;
    double m_width = 0.0;
};
/////////////////////////////////////////////////////
/// \brief The GAPolygon class
///
class GAPolygon : public GerberAperture {
public:
    GAPolygon(double diam, int nVertices, double rotation, double drillDiam)
    {
        m_diam = diam;
        m_verticesCount = nVertices;
        m_rotation = rotation;
        m_drillDiam = drillDiam;
    }
    double rotation() const { return m_rotation; }
    int verticesCount() const { return m_verticesCount; }
    // GerberAperture interface
    QString name() override { return QString("POLYGON(D%1, N%2)").arg(m_diam).arg(m_verticesCount); }
    APERTURE_TYPE type() const override { return POLYGON; }
protected:
    void draw() override
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

private:
    double m_diam = 0.0;
    double m_rotation = 0.0;
    int m_verticesCount = 0;
};
/////////////////////////////////////////////////////
/// \brief The GAMacro class
///
class GAMacro : public GerberAperture {
public:
    GAMacro(const QString& macro, const QList<QString>& modifiers, const QMap<QString, double>& macroCoefficients)
    {
        m_macro = macro;
        m_modifiers = modifiers;
        while (m_modifiers.last().isEmpty()) {
            m_modifiers.removeLast();
        }
        m_macroCoefficients = macroCoefficients;
    }
    // GerberAperture interface
    QString name() override { return QString("MACRO(%1)").arg(m_macro); }
    APERTURE_TYPE type() const override { return APERTURE_MACRO; }
protected:
    void draw() override
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
                for (int j = 0; j <= int(mod[2]); ++j) {
                    polygon.push_back(IntPoint(mod[j * 2 + 3] * uScale, mod[j * 2 + 4] * uScale));
                }
                if (mod.size() > (int(mod[2]) * 2 + 5) && mod.last() > 0) {
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

private:
    QList<QString> m_modifiers;
    QMap<QString, double> m_macroCoefficients;
    QString m_macro;
};

#endif // GERBERAPERTURE_H
