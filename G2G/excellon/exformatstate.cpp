#include "exformatstate.h"

using namespace Excellon;

///////////////////////////////////////////////////////
/// \brief State::reset
/// \param f
///
void State::reset(Format* f)
{
    format = f;
    if (format) {
        format->unitMode = Millimeters;
        format->decimal = 4;
        format->integer = 3;
    }
    rawPos.first.clear();
    rawPos.second.clear();
    gCode = G_NULL;
    mCode = M_NULL;
    tCode = 0;
    pos = QPointF();
    path.clear();
    line = 0;
}
///////////////////////////////////////////////////////
/// \brief State::updatePos
///
void State::updatePos()
{
    pos = QPointF(parseNumber(rawPos.first), parseNumber(rawPos.second));
    for (int i = 0; i < rawPosList.size(); ++i) {
        path[i] = QPointF(parseNumber(rawPosList[i].first), parseNumber(rawPosList[i].second));
    }
}
///////////////////////////////////////////////////////
/// \brief State::parseNumber
/// \param Str
/// \return
///
double State::parseNumber(QString Str)
{
    double val = 0.0;
    int sign = +1;
    if (!Str.isEmpty()) {
        if (Str.contains('.')) {
            val = Str.toDouble();
        } else {

            if (Str.startsWith('+')) {
                Str.remove(0, 1);
                sign = +1;
            } else if (Str.startsWith('-')) {
                Str.remove(0, 1);
                sign = -1;
            }
            if (Str.length() < format->integer + format->decimal) {
                switch (format->zeroMode) {
                case LeadingZeros:
                    Str = Str + QString(format->integer + format->decimal - Str.length(), '0');
                    break;
                case TrailingZeros:
                    Str = QString(format->integer + format->decimal - Str.length(), '0') + Str;
                    break;
                }
            }
            val = Str.toDouble() * pow(10.0, -format->decimal) * sign;
        }
        if (format->unitMode == Inches)
            val *= 25.4;
    }
    return val;
}
///////////////////////////////////////////////////////
/// \brief State::currentToolDiameter
/// \return
///
double State::currentToolDiameter() const
{
    return format->file->tool(tCode);
}
