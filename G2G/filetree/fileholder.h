#ifndef FileHolder_H
#define FileHolder_H

#include <QMap>
#include <QMutex>
#include <file.h>
#include <gcode/drl.h>
#include <gcode/gcode.h>
#include <myclipper.h>

using namespace ClipperLib;

class FileHolder {
public:
    FileHolder();

    template <typename T>
    static T* file(int id)
    {
        QMutexLocker locker(&m_mutex);
        QMap<int, QSharedPointer<T>>* files = reinterpret_cast<QMap<int, QSharedPointer<T>>*>(&m_files);
        return files->value(id).data();
    }

    static void deleteFile(int id);

    template <typename T>
    static int addFile(T* file)
    {
        QMutexLocker locker(&m_mutex);
        const int id = m_id;
        ++m_id;
        QMap<int, QSharedPointer<T>>* files = reinterpret_cast<QMap<int, QSharedPointer<T>>*>(&m_files);
        files->insert(id, QSharedPointer<T>(file));
        return id;
    }

    static bool isEmpty() ;

    static Paths getPaths();

    template <typename T>
    static QVector<T*> files() /*const*/
    {
        QMutexLocker locker(&m_mutex);
        QMap<int, QSharedPointer<T>>* files = reinterpret_cast<QMap<int, QSharedPointer<T>>*>(&m_files);
        QVector<T*> rfiles;
        rfiles.reserve(files->size());
        for (const QSharedPointer<T>& sp : *files) {
            if (dynamic_cast<T*>(sp.data()))
                rfiles.append(sp.data());
        }
        return rfiles;
    }

private:
    static QMutex m_mutex;
    static QMap<int, QSharedPointer<AbstractFile<int>>> m_files;
    static int m_id;
};

#endif // FileHolder_H
