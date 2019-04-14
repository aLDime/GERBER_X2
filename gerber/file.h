#ifndef GFILE_H
#define GFILE_H

#include "aperture.h"
#include "gerber.h"
#include "graphicobject.h"

#include <QDebug>
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
    void grouping(PolyNode* node, Pathss* pathss, Group group)
    {
        Path path;
        Paths paths;
        switch (group) {
        case CutoffGroup:
            if (!node->IsHole()) {
                path = node->Contour;
                paths.push_back(path);
                for (int var = 0; var < node->ChildCount(); ++var) {
                    path = node->Childs[var]->Contour;
                    paths.push_back(path);
                }
                pathss->push_back(paths);
            }
            for (int var = 0; var < node->ChildCount(); ++var) {
                grouping(node->Childs[var], pathss, group);
            }
            break;
        case CopperGroup:
            if (node->IsHole()) {
                path = node->Contour;
                paths.push_back(path);
                for (int var = 0; var < node->ChildCount(); ++var) {
                    path = node->Childs[var]->Contour;
                    paths.push_back(path);
                }
                pathss->push_back(paths);
            }
            for (int var = 0; var < node->ChildCount(); ++var) {
                grouping(node->Childs[var], pathss, group);
            }
            break;
        }
    }
};

Q_DECLARE_METATYPE(File)
}

#endif // GFILE_H
