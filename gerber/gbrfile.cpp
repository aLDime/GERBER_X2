#include "gbrfile.h"
#include <QElapsedTimer>
#include <QSemaphore>
#include <QThread>
#include <settings.h>

using namespace Gerber;

Format* crutch;

File::File(const QString& fileName) { m_name = fileName; }

template <typename T>
void addData(QByteArray& dataArray, const T& data)
{
    dataArray.append(reinterpret_cast<const char*>(&data), sizeof(T));
}

File::~File()
{
}

const QMap<int, QSharedPointer<AbstractAperture>>* const File::apertures() const
{
    return &m_apertures;
}

Paths File::merge() const
{
#ifdef QT_DEBUG
    qDebug() << "merge()" << size();
#endif
    QElapsedTimer t;
    t.start();
    m_mergedPaths.clear();
    int i = 0;
    while (i < size()) {
        Clipper clipper;
        clipper.AddPaths(m_mergedPaths, ptSubject, true);

        const int exp = at(i).state().imgPolarity();

        Paths workingPaths;

        do {
            Paths paths(at(i++).paths());
            workingPaths.append(paths);
        } while (i < size() && exp == at(i).state().imgPolarity());

        if (at(i - 1).state().imgPolarity() == Positive) {
            clipper.AddPaths(workingPaths, ptClip, true);
            clipper.Execute(ctUnion, m_mergedPaths, pftPositive);
        } else {
            clipper.AddPaths(workingPaths, ptClip, true);
            clipper.Execute(ctDifference, m_mergedPaths, pftNonZero);
        }
    }
    if (Settings::cleanPolygons())
        CleanPolygons(m_mergedPaths, 0.0005 * uScale);
#ifdef QT_DEBUG
    qDebug() << shortName() << t.elapsed();
#endif
    return m_mergedPaths;
}

void File::grouping(PolyNode* node, Pathss* pathss, File::Group group)
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

File::ItemsType File::itemsType() const { return m_itemsType; }

void File::setRawItemGroup(ItemGroup* itemGroup) { m_rawItemGroup = QSharedPointer<ItemGroup>(itemGroup); }

ItemGroup* File::rawItemGroup() const { return m_rawItemGroup.data(); }

Pathss& File::groupedPaths(File::Group group, bool fl)
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
}

void Gerber::File::write(QDataStream& stream) const
{
    stream << *this;
    stream << m_apertures;
    stream << m_format;
    stream << layer;
    stream << miror;
    stream << rawIndex;
    stream << m_itemsType;
    _write(stream);
}

void Gerber::File::read(QDataStream& stream)
{
    crutch = &m_format;
    stream >> *this;
    stream >> m_apertures;
    stream >> m_format;
    stream >> (int&)layer;
    stream >> (int&)miror;
    stream >> rawIndex;
    stream >> (int&)m_itemsType;
    for (GraphicObject& go : *this) {
        go.m_gFile = this;
        go.m_state.m_format = format();
    }
    _read(stream);
}

void Gerber::File::createGi()
{
    if (shortName().contains("bot", Qt::CaseInsensitive))
        setSide(Bottom);

    for (Paths& paths : groupedPaths()) {
        GraphicsItem* item = new GerberItem(paths, this);
        item->m_id = m_itemGroup->size();
        m_itemGroup->append(item);
    }
    setRawItemGroup(new ItemGroup);
    if (!Settings::skipDuplicates()) {
        for (const GraphicObject& go : *this) {
            GraphicsItem* item = new RawItem(go.path(), this);
            item->m_id = rawItemGroup()->size();
            m_rawItemGroup->append(item);
        }
    } else {
        QList<Path> checkList;
        for (int i = 0; i < size(); ++i) {
            const GraphicObject& go = at(i);
            if (go.path().size() > 1) { // skip empty
                bool contains = false;
                for (const Path& path : checkList) { // find copy
                    int counter = 0;
                    if (path.size() == go.path().size()) {
                        for (const IntPoint& p1 : path) {
                            for (const IntPoint& p2 : go.path()) {
                                const double k = 0.001 * uScale;
                                if ((abs(p1.X - p2.X) < k) && (abs(p1.Y - p2.Y) < k)) {
                                    ++counter;
                                    break;
                                }
                            }
                        }
                    }
                    if (counter == go.path().size()) {
                        contains = true;
                        break;
                    }
                }
                if (!contains) {
                    checkList.append(go.path());
                    GraphicsItem* item = new RawItem(go.path(), this);
                    item->m_id = rawItemGroup()->size();
                    m_rawItemGroup->append(item);
                }
            }
        }
    }
    if (m_itemsType == Normal)
        m_rawItemGroup->setVisible(false);
    else
        m_itemGroup->setVisible(false);
}

QDataStream& operator<<(QDataStream& stream, const QSharedPointer<AbstractAperture>& m_aperture)
{
    stream << m_aperture->type();
    m_aperture->write(stream);
    return stream;
}

QDataStream& operator>>(QDataStream& stream, QSharedPointer<AbstractAperture>& m_aperture)
{
    int type;
    stream >> type;
    switch (type) {
    case Circle:
        m_aperture = QSharedPointer<AbstractAperture>(new ApCircle(0.0, 0.0, crutch));
        break;
    case Rectangle:
        m_aperture = QSharedPointer<AbstractAperture>(new ApRectangle(0.0, 0.0, 0.0, crutch));
        break;
    case Obround:
        m_aperture = QSharedPointer<AbstractAperture>(new ApObround(0.0, 0.0, 0.0, crutch));
        break;
    case Polygon:
        m_aperture = QSharedPointer<AbstractAperture>(new ApPolygon(0.0, 0.0, 0.0, 0.0, crutch));
        break;
    case Macro:
        m_aperture = QSharedPointer<AbstractAperture>(new ApMacro("", {}, {}, crutch));
        break;
    case Block:
        m_aperture = QSharedPointer<AbstractAperture>(new ApBlock(crutch));
        break;
    }
    m_aperture->read(stream);
    return stream;
}
