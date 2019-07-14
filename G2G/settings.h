#ifndef SETTINGS_H
#define SETTINGS_H

#include <QColor>

enum class Colors : unsigned {
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
    static bool skipDuplicates();

    static QString startGCode();
    static QString endGCode();
    static QString gCodeFormat();

protected:
    static QColor m_color[Colors::Count];
    static int m_minCircleSegments;
    static double m_minCircleSegmentLength;
    static bool m_cleanPolygons;
    static bool m_skipDuplicates;

    static QString m_startGCode;
    static QString m_endGCode;
    static QString m_GCode;
};

#endif // SETTINGS_H
