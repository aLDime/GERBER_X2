#include "file.h"
#include <QElapsedTimer>

using namespace G;

File::File(const QString& fileName) { m_fileName = fileName; }

File::~File() {}

Paths File::merge() const
{
    Paths paths;
    Paths tmpPaths;
    int i = 0, exp = -1;
    while (i < size()) {
        Clipper clipper(ioStrictlySimple);
        clipper.AddPaths(paths, ptSubject, true);
        exp = at(i).state.imgPolarity();
        do {
            tmpPaths = at(i++).paths;
            SimplifyPolygons(tmpPaths, pftNonZero);
            clipper.AddPaths(tmpPaths, ptClip, true);
        } while (i < size() && exp == at(i).state.imgPolarity());

        if (at(i - 1).state.imgPolarity() == Positive)
            clipper.Execute(ctUnion, paths, pftPositive);
        else
            clipper.Execute(ctDifference, paths, pftNonZero);
    }
    CleanPolygons(paths, 0.0009 * uScale);
    m_mergedPaths = paths;
    return m_mergedPaths;
}

File::ItemsType File::itemsType() const { return m_itemsType; }

void File::setRawItemGroup(ItemGroup* itemGroup) { m_rawItemGroup = QSharedPointer<ItemGroup>(itemGroup); }

ItemGroup* File::rawItemGroup() const { return m_rawItemGroup.data(); }

bool File::flashedApertures() const
{
    for (QSharedPointer<AbstractAperture> a : m_apertures) {
        if (a.data()->isFlashed())
            return true;
    }
    return false;
}

ItemGroup* File::itemGroup() const
{
    if (m_itemsType == Normal)
        return m_itemGroup.data();
    else
        return m_rawItemGroup.data();
}

void File::setItemType(File::ItemsType type)
{
    bool visible;
    if (m_itemsType == Normal)
        visible = m_itemGroup.data()->isVisible();
    else
        visible = m_rawItemGroup.data()->isVisible();

    m_itemsType = type; // !!!

    if (m_itemsType == Normal && visible) {
        m_itemGroup.data()->setVisible(visible);
        m_rawItemGroup.data()->setVisible(false);
    } else {
        m_itemGroup.data()->setVisible(false);
        m_rawItemGroup.data()->setVisible(visible);
    }

    //    if (m_type == Normal)
    //        return m_itemGroup;
    //    else
    //        return m_rawItemGroup.data();
}

QMap<int, QSharedPointer<AbstractAperture>> File::apertures() const { return m_apertures; }

//Pathss& File::groupedPaths(Group group, bool fl)
//{
//    PolyTree polyTree;
//    Clipper clipper;
//    clipper.AddPaths(m_mergedPaths, ptSubject, true);
//    IntRect r(clipper.GetBounds());
//    int k = /*uScale*/ 1;
//    Path outer = {
//        IntPoint(r.left - k, r.bottom + k),
//        IntPoint(r.right + k, r.bottom + k),
//        IntPoint(r.right + k, r.top - k),
//        IntPoint(r.left - k, r.top - k)
//    };
//    if (fl)
//        ReversePath(outer);
//    clipper.AddPath(outer, ptSubject, true);
//    clipper.Execute(ctUnion, polyTree, pftNonZero);
//    grouping(polyTree.GetFirst(), &m_groupedPaths, group);
//    return m_groupedPaths;
//}

//void File::grouping(PolyNode* node, Pathss* pathss, File::Group group)
//{
//    Path path;
//    Paths paths;
//    switch (group) {
//    case CutoffGroup:
//        if (!node->IsHole()) {
//            path = node->Contour;
//            paths.push_back(path);
//            for (int var = 0; var < node->ChildCount(); ++var) {
//                path = node->Childs[var]->Contour;
//                paths.push_back(path);
//            }
//            pathss->push_back(paths);
//        }
//        for (int var = 0; var < node->ChildCount(); ++var) {
//            grouping(node->Childs[var], pathss, group);
//        }
//        break;
//    case CopperGroup:
//        if (node->IsHole()) {
//            path = node->Contour;
//            paths.push_back(path);
//            for (int var = 0; var < node->ChildCount(); ++var) {
//                path = node->Childs[var]->Contour;
//                paths.push_back(path);
//            }
//            pathss->push_back(paths);
//        }
//        for (int var = 0; var < node->ChildCount(); ++var) {
//            grouping(node->Childs[var], pathss, group);
//        }
//        break;
//    }
//}
