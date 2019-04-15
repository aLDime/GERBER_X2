#ifndef STATE_H
#define STATE_H
#include "excellon.h"

namespace Excellon {

///////////////////////////////////////////////////////
/// \brief The Format struct
///
struct Format {
    Format(DrillFile* file = nullptr)
        : file(file)
    {
    }
    ZeroMode zeroMode = LeadingZeros;
    UnitMode unitMode = Millimeters;
    int decimal = 0;
    int integer = 0;
    QPointF offsetPos;
    DrillFile* /*const*/ file = nullptr;
};

///////////////////////////////////////////////////////
/// \brief The State struct
///
struct State {
    double currentToolDiameter() const;
    double parseNumber(QString Str);
    void reset(Format* f);
    void updatePos();

    QPair<QString, QString> rawPos;
    QList<QPair<QString, QString>> rawPosList;
    Format* format = nullptr;
    GCode gCode = G_NULL;
    MCode mCode = M_NULL;
    int tCode = -1;
    QPointF pos;
    QPointF offsetPos() const { return pos + format->offsetPos; }
    QPolygonF path;
    int line = 0;
};

} // namespace Excellon
#endif // STATE_H
