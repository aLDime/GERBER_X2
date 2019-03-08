#ifndef GFILE_H
#define GFILE_H

#include "aperture.h"
#include "gerber.h"
#include "graphicobject.h"

#include <abstractfile.h>
#include <gi/itemgroup.h>

namespace G {

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
    FileType type() const override { return FileType::Gerber; }

    QMap<int, QSharedPointer<AbstractAperture>> apertures() const;
    bool flashedApertures() const;

protected:
    Paths merge() const override;

private:
    QMap<int, QSharedPointer<AbstractAperture>> m_apertures;
};

Q_DECLARE_METATYPE(File)
}

#endif // GFILE_H
