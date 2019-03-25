#include "abstractfile.h"

//Paths AbstractFile::merge()
//{
//    Paths paths;
//    Paths tmpPaths;
//    //    AbstractFile = gerberAbstractFile;
//    int i = 0, exp = -1;
//    Clipper clipper(ioStrictlySimple);
//    while (i < size()) {
//        clipper.Clear();
//        clipper.AddPaths(paths, ptSubject, true);
//        exp = at(i).state.imgPolarity;
//        do {
//            tmpPaths = at(i++).paths;
//            SimplifyPolygons(tmpPaths, pftNonZero);
//            clipper.AddPaths(tmpPaths, ptClip, true);
//        } while (i < size() && exp == at(i).state.imgPolarity);
//        if (at(i - 1).state.imgPolarity == G::Positive) {
//            clipper.Execute(ctUnion, paths, pftPositive);
//        } else {
//            clipper.Execute(ctDifference, paths, pftNonZero);
//        }
//    }
//    m_mergedPaths = paths;
//    return m_mergedPaths;
//}

AbstractFile::AbstractFile() {}

AbstractFile::~AbstractFile()
{
    //qDebug("~AbstractFile()");
    //    qDeleteAll(apertures);
    //    if (itemGroup)
    //        delete itemGroup;
}

QString AbstractFile::shortFileName() const { return QFileInfo(m_fileName).fileName(); }

QString AbstractFile::fileName() const { return m_fileName; }

void AbstractFile::setFileName(const QString& fileName) { m_fileName = fileName; }

ItemGroup* AbstractFile::itemGroup() const { return m_itemGroup.data(); }

void AbstractFile::setItemGroup(ItemGroup* itemGroup) { m_itemGroup = QSharedPointer<ItemGroup>(itemGroup); }

Paths AbstractFile::mergedPaths() const { return m_mergedPaths.isEmpty() ? merge() : m_mergedPaths; }

Pathss AbstractFile::groupedPaths() const { return m_groupedPaths; }

QList<QString>& AbstractFile::lines() { return m_lines; }



Side AbstractFile::side() const { return m_side; }

void AbstractFile::setSide(Side side) { m_side = side; }

QColor AbstractFile::color() const { return m_color; }

void AbstractFile::setColor(const QColor& color) { m_color = color; }


