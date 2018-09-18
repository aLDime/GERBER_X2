#ifndef GAbstractFile_H
#define GAbstractFile_H

#include <QFileInfo>
#include <myclipper.h>

#include <gi/itemgroup.h>

using namespace ClipperLib;
enum class FileType {
    Gerber,
    Drill,
    GCode,
};

//template <typename T = int>
class AbstractFile { // : public QList<T> {
public:
    AbstractFile() {}
    ~AbstractFile()
    {
        //qDebug("~AbstractFile()");
        //    qDeleteAll(apertures);
        //    if (itemGroup)
        //        delete itemGroup;
    }

    QString shortFileName() const { return QFileInfo(m_fileName).fileName(); }
    QString fileName() const { return m_fileName; }
    void setFileName(const QString& fileName) { m_fileName = fileName; }

    ItemGroup* itemGroup() const { return m_itemGroup.data(); }
    void setItemGroup(ItemGroup* itemGroup) { m_itemGroup = QSharedPointer<ItemGroup>(itemGroup); }

    Paths mergedPaths() const { return m_mergedPaths.isEmpty() ? merge() : m_mergedPaths; }
    Pathss groupedPaths() const { return m_groupedPaths; }

    QList<QString>& lines() { return m_lines; }

    enum Group {
        CopperGroup,
        CutoffGroup,
    };

    Pathss& groupedPaths(Group group = CopperGroup, bool fl = false)
    {
        if (m_groupedPaths.isEmpty()) {
            PolyTree polyTree;
            Clipper clipper;
            clipper.AddPaths(mergedPaths(), ptSubject, true);
            IntRect r(clipper.GetBounds());
            int k = /*uScale*/ 1;
            Path outer = {
                IntPoint(r.left - k, r.bottom + k),
                IntPoint(r.right + k, r.bottom + k),
                IntPoint(r.right + k, r.top - k),
                IntPoint(r.left - k, r.top - k)
            };
            if (fl)
                ReversePath(outer);
            clipper.AddPath(outer, ptSubject, true);
            clipper.Execute(ctUnion, polyTree, pftNonZero);
            grouping(polyTree.GetFirst(), &m_groupedPaths, group);
        }
        return m_groupedPaths;
    }

    virtual FileType type() const = 0;

protected:
    virtual Paths merge() const = 0;

    QSharedPointer<ItemGroup> m_itemGroup;
    QList<QString> m_lines;
    QString m_fileName;
    mutable Paths m_mergedPaths;
    Pathss m_groupedPaths;

private:
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

//Q_DECLARE_METATYPE(AbstractFile)

#endif // GAbstractFile_H
