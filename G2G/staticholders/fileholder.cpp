#include "FileHolder.h"

//QMap<int, QSharedPointer<Drill>> FileHolder::m_drlFiles;
//QMap<int, QSharedPointer<G::File>> FileHolder::m_gerberFiles;
//QMap<int, QSharedPointer<GCode>> FileHolder::m_gCode;

//template <typename T>
QMap<int, QSharedPointer<AbstractFile>> FileHolder::m_files;
int FileHolder::m_id = 0;
QMutex FileHolder::m_mutex;

FileHolder::FileHolder()
{
}

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
        AbstractFile* item = sp.data();
        if (item && item->itemGroup()->isVisible() && (item->type() == FileType::Gerber || item->type() == FileType::Drill))
            for (Paths& p : item->groupedPaths())
                paths.append(p);
    }
    return paths;
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
