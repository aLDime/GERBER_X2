#ifndef GERBER_H
#define GERBER_H

#include "../clipper/myclipper.h"

#include <QObject>

//#define DEPRECATED
#define DEPRECATED_IMAGE_POLARITY

using namespace ClipperLib;
namespace G {
enum ZERO_OMISSION_MODE {
    OMIT_LEADING_ZEROS,
#ifdef DEPRECATED
    OMIT_TRAILING_ZEROS,
#endif
};

enum UNIT_MODE {
    INCHES,
    MILLIMETERS,
};

enum IMAGE_POLARITY {
    POSITIVE,
#ifdef DEPRECATED_IMAGE_POLARITY
    NEGATIVE,
#endif

};

enum COORDINATE_VALUES_NOTATION {
    ABSOLUTE_NOTATION,
#ifdef DEPRECATED
    INCREMENTAL_NOTATION,
#endif
};

enum INTERPOLATION_MODE {
    LINEAR = 1,
    CLOCKWISE_CIRCULAR = 2,
    COUNTERCLOCKWISE_CIRCULAR = 3
};

enum REGION_MODE {
    OFF,
    ON
};

enum QUADRANT_MODE {
    UNDEF,
    SINGLE,
    MULTI
};

enum D_CODE {
    D01 = 1,
    D02 = 2,
    D03 = 3
};

enum LAYER {
    TOP_ASSY,
    TOP_SILK,
    TOP_PASTE,
    TOP_MASK,
    TOP,
    BOT,
    BOT_MASK,
    BOT_PASTE,
    BOT_SILK,
    BOT_ASSY,
    BOARD,
};

enum G_CODE {
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

enum ATTRIBUTE_TYPE {
    ATTRIBUTE, // TF
    APERTURE_ATTRIBUTE, // TA
    OBJECT_ATTRIBUTE, // TO
    DELETE_ATTRIBUTE // TD
};

enum {
    STEPS_PER_CIRCLE = 72
};

enum PRIMITIVE_TYPE {
    APERTURE,
    LINE,
    REGION,
};

class Format {
public:
    UNIT_MODE unitMode = MILLIMETERS;

    // Warning: Trailing zero omission is deprecated
    ZERO_OMISSION_MODE zeroOmisMode = OMIT_LEADING_ZEROS;

    // Warning: Currently the only allowed notation is absolute notation. The incremental notation is deprecated.
    COORDINATE_VALUES_NOTATION coordValueNotation = ABSOLUTE_NOTATION;

    // Warning: Using less than 4 decimal places is deprecated.
    int xDecimal = 4;
    int xInteger = 3;
    // Warning: Using less than 4 decimal places is deprecated.
    int yDecimal = 4;
    int yInteger = 3;
};

class State {
public:
    //    COORDINATE_VALUES_NOTATION coordValueNotation = ABSOLUTE_NOTATION;
    //    UNIT_MODE unitMode = MILLIMETERS;
    //    ZERO_OMISSION_MODE zeroOmisMode = OMIT_LEADING_ZEROS;
    void reset()
    {
        curDCode = D02;
        format = Format();
        curGCode = G01;
        imgPolarity = POSITIVE;
        curAperture = 0;
        lineNum = 0;
        lstAperture = 0;
        interpolation = LINEAR;
        curPos = IntPoint();
        quadrant = UNDEF;
        region = OFF;
        type = APERTURE;
    }
    D_CODE curDCode = D02;
    G_CODE curGCode = G01;
    IMAGE_POLARITY imgPolarity = POSITIVE;
    INTERPOLATION_MODE interpolation = LINEAR;
    PRIMITIVE_TYPE type = APERTURE;
    QUADRANT_MODE quadrant = UNDEF;
    REGION_MODE region = OFF;

    Format format;

    int curAperture = 0;
    int lineNum = 0;
    int lstAperture = 0;

    IntPoint curPos;
};
}

#endif //   GERBER_H
