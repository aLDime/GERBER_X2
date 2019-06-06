#ifndef VARS_H
#define VARS_H

#include <QDataStream>
#include <QPolygonF>

class DrillItem;

namespace Excellon {

enum UnitMode {
    Inches,
    Millimeters,
};

enum ZeroMode {
    LeadingZeros,
    TrailingZeros,
};

enum MCode {
    M_NULL = -1,
    M00 = 0, //  End of Program - No Rewind (X#Y#)
    M01 = 1, //  End of Pattern
    M02 = 2, //  Repeat Pattern Offset ((M02)X#Y#)
    //    M02 = 2, //  Swap Axes ((M02)XYM70)
    //    M02 = 2, //  Mirror Image X Axis ((M02)XYM80)
    //    M02 = 2, //  Mirror Image Y Axis ((M02)XYM90)
    M06 = 6, //  Optional Stop (X#Y#)
    M08 = 8, //  End of Step and Repeat
    M09 = 9, //  Stop for Inspection (X#Y#)
    M14 = 14, // Z Axis Route Position With Depth Controlled Contouring
    M15 = 15, // Z Axis Route Position
    M16 = 16, // Retract With Clamping - вытянуть с фиксацией.
    M17 = 17, // Retract Without Clamping
    M18 = 18, // Command tool tip check
    M25 = 25, // Beginning of Pattern
    M30 = 30, // End of Program Rewind (X#Y#)
    M45 = 45, // Long Operator message on multiple\ part program lines (,long message\)
    M47 = 47, // Operator Message (,text)
    M48 = 48, // The beginning of a header
    M50 = 50, // Vision Step and Repeat Pattern Start (,#)
    M51 = 51, // Vision Step and Repeat Rewind (,#)
    M52 = 52, // Vision Step and Repeat Offset Counter Control (#)
    M60 = 60, // Reference Scaling enable
    M61 = 61, // Reference Scaling disable
    M62 = 62, // Turn on peck drilling
    M63 = 63, // Turn off peck drilling
    M71 = 71, // Metric Measuring Mode
    M72 = 72, // Inch Measuring Mode
    M95 = 95, // % end header
    M97 = 97, // Canned Text (,text)
    M98 = 98, // Canned Text (,text)
    M99 = 99, // User Defined Stored Pattern (,subprogram)
};

enum GCode {
    G_NULL = -1,
    G00 = 0, //  Route Mode (X#Y#) перемещение.
    G01 = 1, //  Linear (Straight Line) Mode
    G02 = 2, //  Circular CW Mode
    G03 = 3, //  Circular CCW Mode
    G04 = 4, //  X# Variable Dwell
    G05 = 5, //  Drill Mode
    G07 = 7, //  Override current tool feed or speed
    G32 = 32, // Routed Circle Canned Cycle (X#Y#A#)
    // CCW G34,#(,#)	Select Vision Tool
    // CW G33X#Y#A#	Routed Circle Canned Cycle
    G35 = 35, // Single Point Vision Offset (Relative to Work Zero)  (X#Y#)
    G36 = 36, // Multipoint Vision Translation (Relative to Work Zero)  (X#Y#)
    G37 = 37, // Cancel Vision Translation or Offset (From G35 or G36)
    G38 = 38, // Vision Corrected Single Hole Drilling (Relative to Work Zero)  (X#Y#)
    G39 = 39, // Vision System Autocalibration  (X#Y#)
    G40 = 40, // Cutter Compensation Off
    G41 = 41, // Cutter Compensation Left
    G42 = 42, // Cutter Compensation Right
    G45 = 45, // Single Point Vision Offset (Relative to G35 or G36)  (X#Y#)
    G46 = 46, // Multipoint Vision Translation (Relative to G35 or G36)  (X#Y#)
    G47 = 47, // Cancel Vision Translation or Offset (From G45 or G46)
    G48 = 48, // Vision Corrected Single Hole Drilling (Relative to G35 or G36)  (X#Y#)
    G82 = 82, // Dual In Line Package  (G81)
    G83 = 83, // Eight Pin L Pack
    G84 = 84, // Circle
    G85 = 85, // Slot
    G87 = 87, // Routed Step Slot Canned Cycle
    G90 = 90, // Absolute Mode
    G91 = 91, // Incremental Input Mode
    G93 = 93, // Zero Set (X#Y#)
};

/*
G90// Absolute Mode
G05//  Drill Mode
T04
G00X0022665Y-021561  // Route Mode (X#Y#) перемещение.
M15                  // Z Axis Route Position
G01Y-0212854         // Linear (Straight Line) Mode
M16                  // Retract With Clamping
G00X0055735Y-0212854 // Route Mode (X#Y#) перемещение.
M15                  // Z Axis Route Position
G01Y-021561          // Linear (Straight Line) Mode
M16                  // Retract With Clamping
G00X0136221Y0297752  // Route Mode (X#Y#) перемещение.
M15                  // Z Axis Route Position
G01Y0300508          // Linear (Straight Line) Mode
M16                  // Retract With Clamping
G00X0163779Y0300508  // Route Mode (X#Y#) перемещение.
M15                  // Z Axis Route Position
G01Y0297752          // Linear (Straight Line) Mode
M16                  // Retract With Clamping
T05
G00X003044Y-0202814  // Route Mode (X#Y#) перемещение.
M15                  // Z Axis Route Position
G01Y-0202027         // Linear (Straight Line) Mode
M16                  // Retract With Clamping
G00X004796Y-0202027  // Route Mode (X#Y#) перемещение.
M15                  // Z Axis Route Position
G01Y-0202814         // Linear (Straight Line) Mode
M16                  // Retract With Clamping
T07
G00X0100247Y-0195311 // Route Mode (X#Y#) перемещение.
M15                  // Z Axis Route Position
G01Y-0201217         // Linear (Straight Line) Mode
M16                  // Retract With Clamping
G00X0163554Y-0195311 // Route Mode (X#Y#) перемещение.
M15                  // Z Axis Route Position
G01Y-0201217         // Linear (Straight Line) Mode
M16                  // Retract With Clamping
M17
M30
*/

class File;

#pragma pack(push, 1)

struct Format {
    Format(File* file = nullptr);
    ZeroMode zeroMode = LeadingZeros;
    UnitMode unitMode = Millimeters;
    int decimal = 0;
    int integer = 0;
    QPointF offsetPos;
    File* /*const*/ file = nullptr;

    friend QDataStream& operator<<(QDataStream& stream, const Excellon::Format& fmt)
    {
        stream << fmt.zeroMode;
        stream << fmt.unitMode;
        stream << fmt.decimal;
        stream << fmt.integer;
        stream << fmt.offsetPos;
        return stream;
    }
    friend QDataStream& operator>>(QDataStream& stream, Excellon::Format& fmt)
    {
        stream >> (int&)(fmt.zeroMode);
        stream >> (int&)(fmt.unitMode);
        stream >> fmt.decimal;
        stream >> fmt.integer;
        stream >> fmt.offsetPos;
        return stream;
    }
};

#pragma pack(pop)

struct State {
    double currentToolDiameter() const;
    //    double parseNumber(QString Str)
    //    {
    //        double val = 0.0;
    //        int sign = +1;
    //        if (!Str.isEmpty()) {
    //            if (Str.contains('.')) {
    //                val = Str.toDouble();
    //            } else {
    //                if (Str.startsWith('+')) {
    //                    Str.remove(0, 1);
    //                    sign = +1;
    //                } else if (Str.startsWith('-')) {
    //                    Str.remove(0, 1);
    //                    sign = -1;
    //                }
    //                if (Str.length() < format->integer + format->decimal) {
    //                    switch (format->zeroMode) {
    //                    case LeadingZeros:
    //                        Str = Str + QString(format->integer + format->decimal - Str.length(), '0');
    //                        break;
    //                    case TrailingZeros:
    //                        Str = QString(format->integer + format->decimal - Str.length(), '0') + Str;
    //                        break;
    //                    }
    //                }
    //                val = Str.toDouble() * pow(10.0, -format->decimal) * sign;
    //            }
    //            if (format->unitMode == Inches)
    //                val *= 25.4;
    //        }
    //        return val;
    //    }
    void reset(Format* f);
    void updatePos();

    QPair<QString, QString> rawPos;
    QList<QPair<QString, QString>> rawPosList;
    Format* format = nullptr;
    GCode gCode = G_NULL;
    MCode mCode = M_NULL;
    int tCode = -1;
    QPointF pos;
    QPointF offsetedPos() const { return pos + format->offsetPos; }
    QPolygonF path;
    int line = 0;

    friend QDataStream& operator<<(QDataStream& stream, const Excellon::State& stt)
    {
        stream << stt.rawPos;
        stream << stt.rawPosList;
        stream << stt.gCode;
        stream << stt.mCode;
        stream << stt.tCode;
        stream << stt.pos;
        stream << stt.path;
        stream << stt.line;
        return stream;
    }
    friend QDataStream& operator>>(QDataStream& stream, Excellon::State& stt)
    {
        stream >> stt.rawPos;
        stream >> stt.rawPosList;
        stream >> (int&)(stt.gCode);
        stream >> (int&)(stt.mCode);
        stream >> stt.tCode;
        stream >> stt.pos;
        stream >> stt.path;
        stream >> stt.line;
        return stream;
    }
};

class Hole {
public:
    Hole() {}
    Hole(const State& state, File* file)
        : file(file)
        , state(state)
    {
    }

    // QList<T>::node_construct() -> *reinterpret_cast<T*>(n) = t; uses operator=(const Hole&),
    // but it's deleted, because field "file" is "const",
    // so, remove "const"
    //const File* const file = nullptr;
    File* file = nullptr;
    State state;
    DrillItem* item = nullptr;

    friend QDataStream& operator<<(QDataStream& stream, const Excellon::Hole& hole)
    {
        stream << hole.state;
        return stream;
    }
    friend QDataStream& operator>>(QDataStream& stream, Excellon::Hole& hole)
    {
        stream >> hole.state;
        return stream;
    }
    friend QDataStream& readArrayBasedContainer(QDataStream& s, Excellon::Hole& c);
};

} // namespace Excellon

#endif // VARS_H
