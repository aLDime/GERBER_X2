#include "FileHolder.h"

//QMap<int, QSharedPointer<Drill>> FileHolder::m_drlFiles;
//QMap<int, QSharedPointer<G::File>> FileHolder::m_gerberFiles;
//QMap<int, QSharedPointer<GCode>> FileHolder::m_gCode;

//template <typename T>
QMap<int, QSharedPointer<AbstractFile>> FileHolder::m_files;
int FileHolder::m_id = 0;
QMutex FileHolder::m_mutex;

FileHolder::FileHolder() {}

bool FileHolder::isEmpty()
{
    QMutexLocker locker(&m_mutex);
    for (const QSharedPointer<AbstractFile>& sp : m_files) {
        if (sp.data() && (sp.data()->type() == FileType::Gerber || sp.data()->type() == FileType::Drill))
            return true;
    }
    return false;
}

int FileHolder::size()
{
    return m_files.size();
}

Paths FileHolder::getPaths()
{
    QMutexLocker locker(&m_mutex);
    Paths paths;
    for (const QSharedPointer<AbstractFile>& sp : m_files) {
        AbstractFile* file = sp.data();
        if (file
            && file->itemGroup()->isVisible()
            && (file->type() == FileType::Gerber || file->type() == FileType::Drill))
            for (Paths& p : file->groupedPaths())
                paths.append(p);
    }
    return paths;
}

Paths FileHolder::getSelectedPaths()
{
    QMutexLocker locker(&m_mutex);
    Paths paths;
    for (const QSharedPointer<AbstractFile>& sp : m_files) {
        AbstractFile* file = sp.data();
        if (file
            && file->itemGroup()->isVisible()
            && (file->type() == FileType::Gerber || file->type() == FileType::Drill))
            for (GraphicsItem* item : *file->itemGroup())
                if (item->isSelected())
                    paths.append(item->paths());
    }
    return paths;
}

QRectF FileHolder::getSelectedBoundingRect()
{
    QMutexLocker locker(&m_mutex);
    IntPoint topleft(std::numeric_limits<cInt>::max(), std::numeric_limits<cInt>::max());
    IntPoint bottomRight(std::numeric_limits<cInt>::min(), std::numeric_limits<cInt>::min());
    for (const QSharedPointer<AbstractFile>& filePtr : m_files) {
        if (filePtr->itemGroup()->isVisible()) {
            for (const GraphicsItem* const item : *filePtr->itemGroup()) {
                if (item->isSelected()) {
                    for (const Path& path : item->paths()) {
                        for (const IntPoint& pt : path) {
                            topleft.X = qMin(pt.X, topleft.X);
                            topleft.Y = qMin(pt.Y, topleft.Y);
                            bottomRight.X = qMax(pt.X, bottomRight.X);
                            bottomRight.Y = qMax(pt.Y, bottomRight.Y);
                        }
                    }
                }
            }
        }
    }
    return QRectF(toQPointF(topleft), toQPointF(bottomRight));
}

QString FileHolder::fileNames()
{
    QMutexLocker locker(&m_mutex);
    QString paths;
    for (const QSharedPointer<AbstractFile>& sp : m_files) {
        AbstractFile* item = sp.data();
        if (item && (item->type() == FileType::Gerber || item->type() == FileType::Drill))
            paths.append(item->fileName()).append('|');
    }
    return paths;
}

void FileHolder::deleteFile(int id)
{
    QMutexLocker locker(&m_mutex);
    if (m_files.contains(id))
        m_files.take(id);
    else
        qWarning() << "Error id";
}
