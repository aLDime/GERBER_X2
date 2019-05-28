#ifndef SETTINGS_H
#define SETTINGS_H

#include <QColor>

enum class Colors : int {
    Background,
    Shtift,
    CutArea,
    Grid1,
    Grid5,
    Grid10,
    Hole,
    Home,
    ToolPath,
    Zero,
    G0,
    Count
};

class Settings {
public:
    Settings();

    static int circleSegments(double radius);
    static QColor& color(Colors id);
    static bool cleanPolygons();

protected:
    static QColor m_color[(size_t)Colors::Count];
    static int m_minCircleSegments;
    static double m_minCircleSegmentLength;
    static bool m_cleanPolygons;
};

#endif // SETTINGS_H
