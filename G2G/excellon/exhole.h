#ifndef HOLE_H
#define HOLE_H

#include "excellon.h"
namespace Excellon {
///////////////////////////////////////////////////////
/// \brief The Hole class
///
class Hole {
public:
    Hole(const State& state, DrillFile* file)
        : file(file)
        , state(state)
    {
    }
    const DrillFile* const file = nullptr;
    State state;
    DrillItem* item = nullptr;
};
} // namespace Excellon

#endif // HOLE_H
