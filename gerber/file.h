#ifndef GFILE_H
#define GFILE_H

#include "aperture.h"
#include "gerber.h"

#include <abstractfile.h>
#include <gi/itemgroup.h>

namespace G {

class File;

class GraphicObject {
public:
    GraphicObject(
        const State& state,
        const Paths& paths,
        File* gFile,
        const QStringList& /*gerberStrings*/ = QStringList(),
        const Path& path = Path())
        : state(state)
        , paths(paths)
        , gFile(gFile)
        //        , gerberStrings(gerberStrings)
        , path(path)
    {
    }

    State state;
    Paths paths;
    File* gFile = nullptr;
    //    QStringList gerberStrings;
    Path path;
};

class File : public AbstractFile, public QList<GraphicObject> {
    friend class Parser;

public:
    File(const QString& shortFileName = "");
    ~File();

    enum Group {
        CopperGroup,
        CutoffGroup,
    };

    Format format;

    Layer layer = Copper;
    Miror miror = Vertical;
    Side side = Top;
    FileType type() const override { return FileType::Gerber; }

    QMap<int, QSharedPointer<AbstractAperture>> getApertures() const;

protected:
    Paths merge() const override;

private:
    QMap<int, QSharedPointer<AbstractAperture>> apertures;
};

Q_DECLARE_METATYPE(File)
}

#endif // GFILE_H
