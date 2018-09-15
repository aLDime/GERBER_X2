#include "FileHolder.h"

//QMap<int, QSharedPointer<Drill>> FileHolder::m_drlFiles;
//QMap<int, QSharedPointer<G::File>> FileHolder::m_gerberFiles;
//QMap<int, QSharedPointer<GCode>> FileHolder::m_gCode;

//template <typename T>
QMap<int, QSharedPointer<AbstractFile<int>>> FileHolder::m_files;
int FileHolder::m_id = 0;
QMutex FileHolder::m_mutex;

FileHolder::FileHolder()
{
}

bool FileHolder::isEmpty()
{
    QMutexLocker locker(&m_mutex);
    qDebug() << "isEmpty 1" << m_files.size();
    for (const QSharedPointer<G::File>& sp : *reinterpret_cast<QMap<int, QSharedPointer<G::File>>*>(&m_files)) {
        if (dynamic_cast<G::File*>(sp.data())) {
            return true;
        }
    }
    qDebug() << "isEmpty 2" << m_files.size();
    for (const QSharedPointer<Drill>& sp : *reinterpret_cast<QMap<int, QSharedPointer<Drill>>*>(&m_files)) {
        if (dynamic_cast<Drill*>(sp.data())) {
            return true;
        }
    }
    qDebug() << "isEmpty 3" << m_files.size();
    return false;
}

Paths FileHolder::getPaths()
{
    QMutexLocker locker(&m_mutex);
    Paths paths;
    for (const QSharedPointer<G::File>& sp : *reinterpret_cast<QMap<int, QSharedPointer<G::File>>*>(&m_files)) {
        G::File* item = dynamic_cast<G::File*>(sp.data());
        if (item && item->itemGroup()->isVisible()) {
            paths.append(item->mergedPaths());
        }
    }
    for (const QSharedPointer<Drill>& sp : *reinterpret_cast<QMap<int, QSharedPointer<Drill>>*>(&m_files)) {
        Drill* item = dynamic_cast<Drill*>(sp.data());
        if (item && item->itemGroup()->isVisible()) {
            paths.append(item->mergedPaths());
        }
    }
    return paths;
}

void FileHolder::deleteFile(int id)
{
    QMutexLocker locker(&m_mutex);
    if (m_files.contains(id))
        m_files.take(id);
    else
        qDebug() << "Error id";
}
