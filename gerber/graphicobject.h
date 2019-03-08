#ifndef GRAPHICOBJECT_H
#define GRAPHICOBJECT_H

#include "state.h"

namespace G {
class File;

class GraphicObject {
public:
    GraphicObject(
        const State& state,
        const Paths& paths,
        File* gFile,
        const Path& path = Path())
        : state(state)
        , paths(paths)
        , gFile(gFile)
        , path(path)
    {
    }

    State state;
    Paths paths;
    File* gFile = nullptr;
    Path path;
};
}

#endif // GRAPHICOBJECT_H
