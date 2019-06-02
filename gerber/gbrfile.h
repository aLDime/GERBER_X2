#ifndef GFILE_H
#define GFILE_H

#include "gbraperture.h"
#include "gbrvars.h"

#include <QDebug>
#include <abstractfile.h>
#include <gi/itemgroup.h>

namespace Gerber {

class File : public AbstractFile, public QList<GraphicObject> {
    friend class Parser;

public:
    File(const QString& fileName = "");
    ~File();

    enum Group {
        CopperGroup,
        CutoffGroup,
    };

    enum ItemsType {
        Normal,
        Raw,
    };

    Format format;
    Layer layer = Copper;
    Miror miror = Vertical;
    FileType type() const override { return FileType::Gerber; }
    const QMap<int, QSharedPointer<AbstractAperture>>* const apertures() const;
    bool flashedApertures() const;
    ItemGroup* itemGroup() const override;
    void setItemType(ItemsType type);
    ItemsType itemsType() const;
    void setRawItemGroup(ItemGroup* itemGroup);
    ItemGroup* rawItemGroup() const;
    Pathss& groupedPaths(Group group = CopperGroup, bool fl = false);

protected:
    Paths merge() const override;

private:
    QMap<int, QSharedPointer<AbstractAperture>> m_apertures;
    ItemsType m_itemsType = Normal;
    QSharedPointer<ItemGroup> m_rawItemGroup;
    void grouping(PolyNode* node, Pathss* pathss, Group group);

    // AbstractFile interface
public:
    virtual void save() const override;
    virtual void open() const override;
};
}

Q_DECLARE_METATYPE(Gerber::File)

#endif // GFILE_H
