#ifndef GERBER_H
#define GERBER_H

#include <QObject>
#include <myclipper.h>

#define DEPRECATED

namespace Gerber {

enum ZeroOmissionMode {
    OmitLeadingZeros,
#ifdef DEPRECATED
    OmitTrailingZeros,
#endif
};

enum UnitMode {
    Inches,
    Millimeters,
};

enum ImagePolarity {
    Positive,
    Negative,
};

enum CoordinateValuesNotation {
    AbsoluteNotation,
#ifdef DEPRECATED
    IncrementalNotation,
#endif
};

enum InterpolationMode {
    Linear = 1,
    ClockwiseCircular = 2,
    CounterclockwiseCircular = 3
};

enum RegionMode {
    Off,
    On
};

enum QuadrantMode {
    Undef,
    Single,
    Multi
};

enum DCode {
    D01 = 1,
    D02 = 2,
    D03 = 3
};

enum Layer {
    Assy,
    Silk,
    Paste,
    Mask,
    Copper,
    Board,
};

enum Miror {
    Vertical,
    Horizontal
};

enum Mirroring {
    NoMirroring,
    X_Mirroring,
    Y_Mirroring,
    XY_Mirroring,
};

enum GCode {
    // Graphics state operators defining the interpolateparameter defining the interpolate operation (D01).
    G01 = 1, // Sets the interpolation mode tolinear
    G02 = 2, // Sets the interpolation mode to ‘Clockwise circular interpolation’
    G03 = 3, // Sets the interpolation mode to ‘Counterclockwise circularinterpolation’
    // Comment
    G04 = 4,
    // Region mode
    G36 = 36, // Begin region
    G37 = 37, // End region
#ifdef DEPRECATED
    // Change aperture
    G54 = 54,
    // Units mode
    G70 = 70, // Inces
    G71 = 71, // Millimeteres
#endif
    // Graphics state operators defining the quadrant modeparameter, amodifier of the circular interpolation mode.
    G74 = 74, // Sets quadrant mode to ’Singlequadrant’
    G75 = 75, // Sets quadrant mode to ’Multiquadrant’
#ifdef DEPRECATED
    // Absolute / relative coordinates
    G90 = 90, // Absolute
    G91 = 91, // Relative (incremental)
#endif
};

enum AttributeType {
    AttributeA, // TF
    ApertureAttribute, // TA
    ObjectAttribute, // TO
    DeleteAttribute // TD
};

enum PrimitiveType {
    Aperture,
    Line,
    Region,
};

class File;

struct Format {
    UnitMode unitMode = Millimeters;

    // Warning: Trailing zero omission is deprecated
    ZeroOmissionMode zeroOmisMode = OmitLeadingZeros;

    // Warning: Currently the only allowed notation is absolute notation. The incremental notation is deprecated.
    CoordinateValuesNotation coordValueNotation = AbsoluteNotation;

    // Warning: Using less than 4 decimal places is deprecated.
    int xInteger = 3;
    int xDecimal = 4;
    int yInteger = 3;
    int yDecimal = 4;

    friend QDataStream& operator<<(QDataStream& stream, const Gerber::Format& format)
    {
        //    stream << static_cast<int>(st.m_dCode)
        //           << static_cast<int>(st.m_gCode)
        //           << static_cast<int>(st.m_imgPolarity)
        //           << static_cast<int>(st.m_interpolation)
        //           << static_cast<int>(st.m_type)
        //           << static_cast<int>(st.m_quadrant)
        //           << static_cast<int>(st.m_region)
        //           << st.m_aperture
        //           << st.m_lineNum
        //           << st.m_curPos
        //           << static_cast<int>(st.m_mirroring)
        //           << st.m_scaling
        //           << st.m_rotating;
        stream.writeRawData(reinterpret_cast<const char*>(&format), sizeof(Gerber::Format));
        return stream;
    }

    friend QDataStream& operator>>(QDataStream& stream, Gerber::Format& format)
    {
        //    stream >> static_cast<qint32>(st.m_dCode);
        //    stream >> static_cast<qint32>(st.m_gCode);
        //    stream >> static_cast<qint32>(st.m_imgPolarity);
        //    stream >> static_cast<qint32>(st.m_interpolation);
        //    stream >> static_cast<qint32>(st.m_type);
        //    stream >> static_cast<qint32>(st.m_quadrant);
        //    stream >> static_cast<qint32>(st.m_region);
        //    stream >> st.m_aperture;
        //    stream >> st.m_lineNum;
        //    stream >> st.m_curPos;
        //    stream >> static_cast<int>(st.m_mirroring);
        //    stream >> st.m_scaling;
        //    stream >> st.m_rotating;
        stream.readRawData(reinterpret_cast<char*>(&format), sizeof(Gerber::Format));
        return stream;
    }
};

class State {
    friend class File;
    friend QDataStream& operator<<(QDataStream& stream, const Gerber::State& state)
    {
        //    stream << static_cast<int>(st.m_dCode)
        //           << static_cast<int>(st.m_gCode)
        //           << static_cast<int>(st.m_imgPolarity)
        //           << static_cast<int>(st.m_interpolation)
        //           << static_cast<int>(st.m_type)
        //           << static_cast<int>(st.m_quadrant)
        //           << static_cast<int>(st.m_region)
        //           << st.m_aperture
        //           << st.m_lineNum
        //           << st.m_curPos
        //           << static_cast<int>(st.m_mirroring)
        //           << st.m_scaling
        //           << st.m_rotating;
        stream.writeRawData(reinterpret_cast<const char*>(&state), sizeof(Gerber::State));
        return stream;
    }

    friend QDataStream& operator>>(QDataStream& stream, Gerber::State& state)
    {
        //    stream >> static_cast<qint32>(st.m_dCode);
        //    stream >> static_cast<qint32>(st.m_gCode);
        //    stream >> static_cast<qint32>(st.m_imgPolarity);
        //    stream >> static_cast<qint32>(st.m_interpolation);
        //    stream >> static_cast<qint32>(st.m_type);
        //    stream >> static_cast<qint32>(st.m_quadrant);
        //    stream >> static_cast<qint32>(st.m_region);
        //    stream >> st.m_aperture;
        //    stream >> st.m_lineNum;
        //    stream >> st.m_curPos;
        //    stream >> static_cast<int>(st.m_mirroring);
        //    stream >> st.m_scaling;
        //    stream >> st.m_rotating;
        stream.readRawData(reinterpret_cast<char*>(&state), sizeof(Gerber::State));
        return stream;
    }

    Format* m_format = nullptr;
    DCode m_dCode = D02;
    GCode m_gCode = G01;
    ImagePolarity m_imgPolarity = Positive;
    InterpolationMode m_interpolation = Linear;
    PrimitiveType m_type = Aperture;
    QuadrantMode m_quadrant = Undef;
    RegionMode m_region = Off;
    int m_aperture = 0;
    int m_lineNum = 0;
    IntPoint m_curPos;
    Mirroring m_mirroring = NoMirroring;
    double m_scaling = 1.0;
    double m_rotating = 0.0;

public:
    State(Format* const format = nullptr)
        : m_format(format)
        , m_dCode(D02)
        , m_gCode(G01)
        , m_imgPolarity(Positive)
        , m_interpolation(Linear)
        , m_type(Aperture)
        , m_quadrant(Undef)
        , m_region(Off)
        , m_aperture(0)
        , m_lineNum(0)
        , m_curPos(IntPoint())
        , m_mirroring(NoMirroring)
        , m_scaling(1.0)
        , m_rotating(0.0)
    {
    }

    inline Format* const format() const { return m_format; }

    inline DCode dCode() const { return m_dCode; }
    inline void setDCode(DCode dCode) { m_dCode = dCode; }

    inline GCode gCode() const { return m_gCode; }
    inline void setGCode(GCode gCode) { m_gCode = gCode; }

    inline ImagePolarity imgPolarity() const { return m_imgPolarity; }
    inline void setImgPolarity(ImagePolarity imgPolarity) { m_imgPolarity = imgPolarity; }

    inline InterpolationMode interpolation() const { return m_interpolation; }
    inline void setInterpolation(InterpolationMode interpolation) { m_interpolation = interpolation; }

    inline PrimitiveType type() const { return m_type; }
    inline void setType(PrimitiveType type) { m_type = type; }

    inline QuadrantMode quadrant() const { return m_quadrant; }
    inline void setQuadrant(QuadrantMode quadrant) { m_quadrant = quadrant; }

    inline RegionMode region() const { return m_region; }
    inline void setRegion(RegionMode region) { m_region = region; }

    inline int aperture() const { return m_aperture; }
    inline void setAperture(int aperture) { m_aperture = aperture; }

    inline IntPoint& curPos() { return m_curPos; }
    inline IntPoint curPos() const { return m_curPos; }
    inline void setCurPos(const IntPoint& curPos) { m_curPos = curPos; }

    inline Mirroring mirroring() const { return m_mirroring; }
    inline void setMirroring(Mirroring mirroring) { m_mirroring = mirroring; }

    inline double scaling() const { return m_scaling; }
    inline void setScaling(double scaling) { m_scaling = scaling; }

    inline double rotating() const { return m_rotating; }
    inline void setRotating(double rotating) { m_rotating = rotating; }
};

class GraphicObject {
    friend class File;
    friend QDataStream& operator<<(QDataStream& stream, const Gerber::GraphicObject& go)
    {
        stream << go.m_path;
        stream << go.m_paths;
        stream << go.m_state;
        return stream;
    }
    friend QDataStream& operator>>(QDataStream& stream, Gerber::GraphicObject& go)
    {
        stream >> go.m_path;
        stream >> go.m_paths;
        stream >> go.m_state;
        return stream;
    }

    File* m_gFile;
    Path m_path;
    Paths m_paths;
    State m_state;

public:
    GraphicObject()
        : m_gFile(nullptr)
    {
    }
    GraphicObject(
        int /*id*/,
        const State& state,
        const Paths& paths,
        File* gFile,
        const Path& path = Path())
        : m_state(state)
        , m_paths(paths)
        , m_gFile(gFile)
        , m_path(path)
    {
    }

    inline File* gFile() const { return m_gFile; }
    inline const Path& path() const { return m_path; }
    inline const Paths& paths() const { return m_paths; }
    inline State state() const { return m_state; }
};

struct StepRepeatStr {
    void reset()
    {
        x = 0;
        y = 0;
        i = 0.0;
        j = 0.0;
        storage.clear();
    }
    int x = 0;
    int y = 0;
    double i = 0.0;
    double j = 0.0;
    QList<GraphicObject> storage;
};

} // namespace Gerber

#endif //   GERBER_H
