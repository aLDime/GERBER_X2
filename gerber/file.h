#ifndef GFILE_H
#define GFILE_H

#include "aperture.h"
#include "gerber.h"
#include "graphicsitem.h"
namespace G {
class File;
class GraphicObject {
public:
    GraphicObject(
        const State& state,
        const Paths& paths,
        File* gFile,
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
    File* gFile = nullptr;
    QStringList gerberStrings;
    Path path;
};

class File : public QList<GraphicObject> {
public:
    File();
    ~File();

    enum GROUP {
        COPPER,
        CUTOFF,
    };

    Paths Merge();
    Pathss& GetGroupedPaths(GROUP group = COPPER, bool fl = false);

    ItemGroup* itemGroup = nullptr;
    QList<QString> lines;
    QMap<int, Aperture*> apertures;
    QString fileName;
    Paths mergedPaths;
    Pathss groupedPaths;

private:
    void grouping(PolyNode* node, Pathss* pathss, GROUP group);
};
}

#endif // GFILE_H
