#include "file.h"
#include <QElapsedTimer>
#include <QSemaphore>
#include <QThread>

using namespace G;

class Union : public QThread {
    //    Q_OBJECT
public:
    Union(QSemaphore* semaphore, const Paths& paths)
        : m_semaphore(semaphore)
        , m_paths(paths)
    {
    }
    virtual ~Union() {}

    // QThread interface
    Paths& paths();

protected:
    void run() override
    {
        //qDebug("run()");
        Clipper clipper(ioStrictlySimple);
        clipper.AddPaths(m_paths, ptClip, true);
        clipper.Execute(ctUnion, m_paths, pftPositive);
        m_semaphore->release();
        //qDebug("exit()");
        exit(0);
    }

private:
    Paths m_paths;
    QSemaphore* m_semaphore = nullptr;
};

File::File(const QString& fileName) { m_fileName = fileName; }

template <typename T>
void addData(QByteArray& dataArray, const T& data)
{
    dataArray.append(reinterpret_cast<const char*>(&data), sizeof(T));
}

File::~File()
{
    return;
    qDebug() << "~File()" << shortFileName();
    QByteArray data;
    QFile file(m_fileName.append(".g2"));
    if (file.open(QFile::WriteOnly)) {

        auto appendSize = [&](int size) {
            data.append(reinterpret_cast<const char*>(&size), sizeof(int));
        };
        addData(data, format);
        addData(data, layer);
        addData(data, miror);
        addData(data, m_itemsType);

        appendSize(m_fileName.size());
        data.append(m_fileName.toLocal8Bit());

        data.append(reinterpret_cast<char*>(&m_side), sizeof(Side));

        appendSize(m_color.name(QColor::HexArgb).size());
        data.append(m_color.name(QColor::HexArgb));

        appendSize(m_lines.size());
        for (const QString& line : m_lines) {
            appendSize(line.size());
            data.append(line.toLocal8Bit());
        }

        appendSize(m_mergedPaths.size());
        for (const Path& path : m_mergedPaths) {
            int size = path.size();
            appendSize(size);
            data.append(reinterpret_cast<const char*>(path.data()), size * sizeof(IntPoint));
        }

        appendSize(m_groupedPaths.size());
        for (const Paths& paths : m_groupedPaths) {
            appendSize(paths.size());
            for (const Path& path : paths) {
                int size = path.size();
                appendSize(size);
                data.append(reinterpret_cast<const char*>(path.data()), size * sizeof(IntPoint));
            }
        }
        file.write(data);
    }
}

Paths File::merge() const
{
#ifdef QT_DEBUG
    qDebug() << "merge()" << size();
#endif
    QElapsedTimer t;
    t.start();
    m_mergedPaths.clear();
    Paths tmpPaths;
    int i = 0, exp = -1;
    while (i < size()) {
        Clipper clipper; //(ioStrictlySimple);
        clipper.AddPaths(m_mergedPaths, ptSubject, true);
        exp = at(i).state.imgPolarity();

        Paths workingPaths;

        do {
            tmpPaths = at(i++).paths;
            //SimplifyPolygons(tmpPaths, pftNonZero);
            workingPaths.append(tmpPaths);
        } while (i < size() && exp == at(i).state.imgPolarity());

        if (at(i - 1).state.imgPolarity() == Positive) {
            if (0) {
                QSemaphore semaphore;
                const int threadCount = QThread::idealThreadCount();
                QVector<Union*> unionThreads;
                //                for (int i = 0; i < threadCount; ++i) {
                //                    unionThreads.append(new Union(&semaphore, {}));
                //                }
                //                int i = 0;
                //                while (workingPaths.size()) {
                //                    for (int j = 0; j < 500 && workingPaths.size(); ++j) {
                //                        unionThreads[i]->paths().append(workingPaths.takeLast());
                //                    }
                //                    (++i == 4) ? i = 0 : i = i;
                //                }
                for (int i = 0, j = workingPaths.size() / threadCount; i < threadCount; ++i) {
                    if (i == threadCount - 1)
                        unionThreads.append(new Union(&semaphore, workingPaths.mid(i * j)));
                    else
                        unionThreads.append(new Union(&semaphore, workingPaths.mid(i * j, j)));
                    unionThreads.last()->start();
                    //                    unionThreads[i]->start();
                }
                if (semaphore.tryAcquire(threadCount, 3600000)) {
                    for (Union* unionThread : unionThreads) {
                        clipper.AddPaths(unionThread->paths(), ptClip, true);
                    }
                    clipper.Execute(ctUnion, m_mergedPaths, pftPositive);
                }
                qDeleteAll(unionThreads);
            } else {
                clipper.AddPaths(workingPaths, ptClip, true);
                clipper.Execute(ctUnion, m_mergedPaths, pftPositive);
            }
        } else {
            clipper.AddPaths(workingPaths, ptClip, true);
            clipper.Execute(ctDifference, m_mergedPaths, pftNonZero);
        }
    }
    CleanPolygons(m_mergedPaths, 0.0005 * uScale);
#ifdef QT_DEBUG
    qDebug() << shortFileName() << t.elapsed();
#endif
    return m_mergedPaths;
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

Paths& Union::paths()
{
    return m_paths;
}
