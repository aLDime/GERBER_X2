#ifndef GERBER_H
#define GERBER_H

#include "../clipper/myclipper.h"

#include <QObject>

#define DEPRECATED
#define DEPRECATED_IMAGE_POLARITY

using namespace ClipperLib;
namespace G {
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
#ifdef DEPRECATED_IMAGE_POLARITY
    Negative,
#endif

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

enum StepRepeat {
    SrClose,
    SrOpen
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

enum { StepsPerCircle = 36 /*72*/ };

enum PrimitiveType {
    Aperture,
    Line,
    Region,
};

struct Format {
    UnitMode unitMode = Millimeters;

    // Warning: Trailing zero omission is deprecated
    ZeroOmissionMode zeroOmisMode = OmitLeadingZeros;

    // Warning: Currently the only allowed notation is absolute notation.
    // The incremental notation is deprecated.
    CoordinateValuesNotation coordValueNotation = AbsoluteNotation;

    // Warning: Using less than 4 decimal places is deprecated.
    int xInteger = 3;
    int xDecimal = 4;
    int yInteger = 3;
    int yDecimal = 4;
};

struct State {
    void reset(Format* f)
    {
        format = f;
        dCode = D02;
        gCode = G01;
        imgPolarity = Positive;
        interpolation = Linear;
        type = Aperture;
        quadrant = Undef;
        region = Off;
        aperture = 0;
        lastAperture = 0;
        lineNum = 0;
        curPos = IntPoint();
    }
    Format* format = nullptr;
    DCode dCode = D02;
    GCode gCode = G01;
    ImagePolarity imgPolarity = Positive;
    InterpolationMode interpolation = Linear;
    PrimitiveType type = Aperture;
    QuadrantMode quadrant = Undef;
    RegionMode region = Off;
    int aperture = 0;
    int lastAperture = 0;
    int lineNum = 0;
    IntPoint curPos;
};
}

#endif //   GERBER_H
