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
        AbstractFile* file = m_files.value(id).data();
        return static_cast<T*>(file);
    }

    static void deleteFile(int id);

    template <typename T>
    static int addFile(T* file)
    {
        QMutexLocker locker(&m_mutex);
        const int id = m_id;
        ++m_id;
        m_files.insert(id, QSharedPointer<AbstractFile>(file));
        return id;
    }

    static bool isEmpty();

    static Paths getPaths();

    template <typename T>
    static QVector<T*> files()
    {
        QMutexLocker locker(&m_mutex);
        QVector<T*> rfiles;
        for (const QSharedPointer<AbstractFile>& sp : m_files) {
            T* file = dynamic_cast<T*>(sp.data());
            if (file)
                rfiles.append(file);
        }
        return rfiles;
    }

private:
    static QMutex m_mutex;
    static QMap<int, QSharedPointer<AbstractFile>> m_files;
    static int m_id;
};

#endif // FileHolder_H
