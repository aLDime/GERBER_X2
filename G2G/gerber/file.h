#ifndef GFILE_H
#define GFILE_H

#include "aperture.h"
#include "gerber.h"
#include "graphicsitem.h"
namespace G {
class GFile;
class GraphicObject {
public:
    GraphicObject(
        const State& state,
        const Paths& paths,
        GFile* gFile,
        const QStringList& gerberStrings = QStringList(),
        const Path& path = Path())
        : state(state)
        , paths(paths)
        , gFile(gFile)
        , gerberStrings(gerberStrings)
        , path(path)
    {
    }

    State state;
    Paths paths;
    GFile* gFile = nullptr;
    QStringList gerberStrings;
    Path path;
};

class GFile : public QList<GraphicObject> {
public:
    GFile() {}
    ~GFile()
    {
        qDeleteAll(apertures);
        if (gig)
            delete gig;
    }
    ItemGroup* gig = nullptr;
    QList<QString> lines;
    QMap<int, Aperture*> apertures;
    QString fileName;
};
}
#endif // GFILE_H
