#ifndef GERBERAPERTURE_H
#define GERBERAPERTURE_H

#include "gerber.h"
#include <QMap>

enum APERTURE_TYPE {
    CIRCULAR,
    RECTANGLE,
    OBROUND,
    POLYGON,
    APERTURE_MACRO
};

class GerberAperture {
public:
    GerberAperture();

    Paths Draw(const STATE& state);

    QMap<QString, double>& MacroCoefficients() { return macroCoefficients; }
        double Diameter() const { return diam; }
    double HoleDiameter() const { return holeDiam; }
    bool IsHole() const { return qFuzzyIsNull(holeDiam); }
    double Width() const { return width; }
    double Height() const { return height; }
    QString Macro() const { return macro; }
    //    QMap<QString, double> MacroCoefficients() const { return macroCoefficients; }
    //    QList<QString> Modifiers() const { return modifiers; }
    int nVertices() const { return numVertices; }
    double Rotation() const { return rotation; }
    double Size();
    APERTURE_TYPE Type() const { return type; }

    void SetDiameter(double diam_);
    void SetHoleDiameter(double holeDiam_);
    void SetWidth(double width_);
    void SetHeight(double height_);
    void SetMacro(const QString& macro_);
    void SetMacroCoefficients(const QMap<QString, double>& macroCoefficients_);
    void SetModifiers(const QList<QString>& modifiers_);
    void SetNumVertices(int nVertices_);
    void SetRotation(double rotation_);
    void SetSize(double size_);
    void SetType(APERTURE_TYPE type_);
    Path GetHole(const STATE& state);

private:
    void Draw();

    APERTURE_TYPE type;

    double diam;
    double height;
    double holeDiam;
    double rotation;
    double size;
    double width;

    int lineNum;
    int numVertices;
    QList<QString> modifiers;
    QMap<QString, double> macroCoefficients;
    Path hole;
    QString macro;
    Paths paths;

    Path Circle(double diametr, IntPoint center = IntPoint());
    Path Rect(double width, double height, IntPoint center = IntPoint());
    Path& Rotate(Path& poligon, double angle, IntPoint center = IntPoint());

    void Translate(Path& path, IntPoint pos);

    void DrawCircular();
    void DrawMacro();
    void DrawObround();
    void DrawPolygon();
    void DrawRectangle();
};

#endif // GERBERAPERTURE_H
