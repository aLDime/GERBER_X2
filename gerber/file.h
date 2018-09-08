#ifndef GFILE_H
#define GFILE_H

#include "aperture.h"
#include "gerber.h"
#include "graphicsitem.h"
namespace G {

enum Layer {
    Copper,
    Mask,
    Silk,
    Board,
};

enum Miror {
    Vertical,
    Horizontal
};

enum Side {
    Top,
    Bottom
};

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

    Layer layer = Copper;
    Miror miror = Vertical;
    Side side = Top;

private:
    void grouping(PolyNode* node, Pathss* pathss, GROUP group);
};

Q_DECLARE_METATYPE(File)
}

#endif // GFILE_H
