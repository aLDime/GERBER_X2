#include "settings.h"
#ifndef M_PI
#define M_PI (3.1415926535897932384626433832795)
#endif
const int gridColor = 100;

QColor Settings::m_color[(size_t)Colors::Count]{
    QColor(), //Background
    QColor(255, 255, 0, 120), //Shtift
    QColor(Qt::gray), //CutArea
    QColor(gridColor, gridColor, gridColor, 50), //Grid1
    QColor(gridColor, gridColor, gridColor, 100), //Grid5
    QColor(gridColor, gridColor, gridColor, 200), //Grid10
    QColor(), //Hole
    QColor(0, 255, 0, 120), //Home
    QColor(Qt::black), //ToolPath
    QColor(255, 0, 0, 120), //Zero
    QColor(Qt::red) //G0
};
int Settings::m_minCircleSegments = 36;
double Settings::m_minCircleSegmentLenght = 0.5;
bool Settings::m_cleanPolygons = true;

Settings::Settings() {}

double Settings::minCircleSegmentLenght() { return m_minCircleSegmentLenght; }
int Settings::minCircleSegments() { return m_minCircleSegments; }

int Settings::circleSegments(double radius)
{
    const double length = Settings::minCircleSegmentLenght(); // 0.5; // mm
    const int destSteps = static_cast<int>(M_PI / asin((length * 0.5) / (radius)));
    int intSteps = Settings::minCircleSegments(); //MinStepsPerCircle; //32 aka 10 degres
    while (intSteps < destSteps)
        intSteps <<= 1; // aka *= 2 // resize to desination 0.5 mm rib length
    return intSteps;
}
QColor& Settings::color(Colors id) { return m_color[static_cast<int>(id)]; }
bool Settings::cleanPolygons() { return m_cleanPolygons; }
