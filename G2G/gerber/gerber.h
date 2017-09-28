#ifndef GERBER_H
#define GERBER_H

#include "../clipper/myclipper.h"
#include <QDebug>
#include <QGraphicsItem>

#define DEPRECATED

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
#ifdef DEPRECATED
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

enum G_CODE {
    // Graphics state operators defining the interpolateparameter defining the interpolate operation (D01).
    G01 = 1, // Sets the interpolation mode tolinear
    G02 = 2, // Sets the interpolation mode to ‘Clockwise circular interpolation’
    G03 = 3, // Sets the interpolation mode to ‘Counterclockwise circularinterpolation’
    G04 = 4, // Comment
    // Region mode
    G36 = 36, // Begin region
    G37 = 37, // End region
#ifdef DEPRECATED
    // Change aperture
    /*Deprecated*/ G54 = 54,
    // Units mode
    /*Deprecated*/ G70 = 70, // Inces
    /*Deprecated*/ G71 = 71, // Millimeteres
#endif
    // Graphics state operators defining the quadrant modeparameter, amodifier of the circular interpolation mode.
    G74 = 74, // Sets quadrant mode to ’Singlequadrant’
    G75 = 75, // Sets quadrant mode to ’Multiquadrant’
#ifdef DEPRECATED
    // Absolute / relative coordinates
    /*Deprecated*/ G90 = 90, // Absolute
    /*Deprecated*/ G91 = 91, // Relative (incremental)
#endif
};

enum ATTRIBUTE_TYPE {
    ATTRIBUTE, // TF
    APERTURE_ATTRIBUTE, // TA
    OBJECT_ATTRIBUTE, // TO
    DELETE_ATTRIBUTE // TD
};

enum {
    STEPS_PER_CIRCLE = 90
};

enum PRIMITIVE_TYPE {
    APERTURE,
    LINE,
    REGION,
};

class FORMAT {
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

class STATE {
public:
    //    COORDINATE_VALUES_NOTATION coordValueNotation = ABSOLUTE_NOTATION;
    //    UNIT_MODE unitMode = MILLIMETERS;
    //    ZERO_OMISSION_MODE zeroOmisMode = OMIT_LEADING_ZEROS;
    D_CODE curDCode = D02;
    FORMAT format;
    G_CODE curGCode = G01;
    IMAGE_POLARITY imgPolarity = POSITIVE;
    int curAperture = 0;
    int lineNum = 0;
    int lstAperture = 0;
    INTERPOLATION_MODE interpolation = LINEAR;
    IntPoint curPos;
    QUADRANT_MODE quadrant = UNDEF;
    REGION_MODE region = OFF;
    PRIMITIVE_TYPE type = APERTURE;
};

#endif //   GERBER_H
