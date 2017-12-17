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
    GACircular(double diam, double drillDiam);
    QString name() override;
    APERTURE_TYPE type() const override;

protected:
    void draw() override;

private:
    double m_diam = 0.0;
};

/////////////////////////////////////////////////////
/// \brief The GARectangle class
///
class GARectangle : public GerberAperture {
public:
    GARectangle(double width, double height, double drillDiam);
    QString name() override;
    APERTURE_TYPE type() const override;

protected:
    void draw() override;

private:
    double m_height = 0.0;
    double m_width = 0.0;
};

/////////////////////////////////////////////////////
/// \brief The GAObround class
///
class GAObround : public GerberAperture {
public:
    GAObround(double width, double height, double drillDiam);
    QString name() override;
    APERTURE_TYPE type() const override;

protected:
    void draw() override;

private:
    double m_height = 0.0;
    double m_width = 0.0;
};

/////////////////////////////////////////////////////
/// \brief The GAPolygon class
///
class GAPolygon : public GerberAperture {
public:
    GAPolygon(double diam, int nVertices, double rotation, double drillDiam);
    double rotation() const;
    int verticesCount() const;

    QString name() override;
    APERTURE_TYPE type() const override;

protected:
    void draw() override;

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
    GAMacro(const QString& macro, const QList<QString>& modifiers, const QMap<QString, double>& macroCoefficients);
    QString name() override;
    APERTURE_TYPE type() const override;

protected:
    void draw() override;

private:
    QList<QString> m_modifiers;
    QMap<QString, double> m_macroCoefficients;
    QString m_macro;
};

#endif // GERBERAPERTURE_H
