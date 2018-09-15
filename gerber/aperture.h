#ifndef GERBERAPERTURE_H
#define GERBERAPERTURE_H

#include "gerber.h"

#include <QMap>
#include <QtMath>
namespace G {

enum ApertureType {
    Circle,
    Rectangle,
    Obround,
    Polygon,
    Macro,
};

class AbstractAperture {
    Q_DISABLE_COPY(AbstractAperture)

public:
    AbstractAperture();
    virtual ~AbstractAperture();

    bool isDrilled() const { return m_drillDiam != 0.0; }
    bool isFlashed() const { return m_isFlashed; }

    double drillDiameter() const { return m_drillDiam; }
    double size();

    Path drawDrill(const State& state);
    Paths draw(const State& state);

    virtual QString name() = 0;
    virtual ApertureType type() const = 0;

protected:
    bool m_isFlashed = false;
    double m_drillDiam = 0.0;
    double m_size = 0.0;

    Paths m_paths;
    virtual void draw() = 0;

    Path circle(double diametr, IntPoint center = IntPoint());
    Path rectangle(double m_width, double m_height, IntPoint center = IntPoint());
    void rotate(Path& poligon, double angle, IntPoint center = IntPoint());
    void translate(Path& path, IntPoint pos);
};

/////////////////////////////////////////////////////
/// \brief The GACircular class
///
class ApCircle : public AbstractAperture {
public:
    ApCircle(double diam, double drillDiam);
    QString name() override;
    ApertureType type() const override;

protected:
    void draw() override;

private:
    double m_diam = 0.0;
};

/////////////////////////////////////////////////////
/// \brief The GARectangle class
///
class ApRectangle : public AbstractAperture {
public:
    ApRectangle(double width, double height, double drillDiam);
    QString name() override;
    ApertureType type() const override;

protected:
    void draw() override;

private:
    double m_height = 0.0;
    double m_width = 0.0;
};

/////////////////////////////////////////////////////
/// \brief The GAObround class
///
class ApObround : public AbstractAperture {
public:
    ApObround(double width, double height, double drillDiam);
    QString name() override;
    ApertureType type() const override;

protected:
    void draw() override;

private:
    double m_height = 0.0;
    double m_width = 0.0;
};

/////////////////////////////////////////////////////
/// \brief The GAPolygon class
///
class ApPolygon : public AbstractAperture {
public:
    ApPolygon(double diam, int nVertices, double rotation, double drillDiam);
    double rotation() const;
    int verticesCount() const;

    QString name() override;
    ApertureType type() const override;

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
class ApMacro : public AbstractAperture {
public:
    ApMacro(const QString& macro, const QList<QString>& modifiers, const QMap<QString, double>& macroCoefficients);
    QString name() override;
    ApertureType type() const override;

protected:
    void draw() override;

private:
    QList<QString> m_modifiers;
    QMap<QString, double> m_macroCoefficients;
    QString m_macro;

    Path drawCenterLine(const QList<double>& mod);
    Path drawCircle(const QList<double>& mod);
    Path drawOutlineCustomPolygon(const QList<double>& mod);
    Path drawOutlineRegularPolygon(const QList<double>& mod);
    Path drawVectorLine(const QList<double>& mod);
    void drawMoire(const QList<double>& mod);
    void drawThermal(const QList<double>& mod);
};
}
#endif // GERBERAPERTURE_H
