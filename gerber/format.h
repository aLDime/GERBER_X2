#ifndef FORMAT_H
#define FORMAT_H

#include "gerber.h"

namespace G {
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
}

#endif // FORMAT_H
