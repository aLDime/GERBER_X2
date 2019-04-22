#ifndef FileHolder_H
#define FileHolder_H

#include <QMap>
#include <QMutex>
#include <exfile.h>
#include <gbrfile.h>
#include <gcode/gcode.h>
#include <myclipper.h>

using namespace ClipperLib;

class FileHolder {
public:
    FileHolder();

    static AbstractFile* file(int id)
    {
        QMutexLocker locker(&m_mutex);
        return m_files.value(id).data();
    }

    template <typename T>
    static T* file(int id)
    {
        QMutexLocker locker(&m_mutex);
        return static_cast<T*>(m_files.value(id).data());
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
    static int size();

    static Paths getPaths();
    static Paths getSelectedPaths();
    static QRectF getSelectedBoundingRect();
    static QString fileNames();

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

    static bool contains(AbstractFile* file)
    {
        return m_files.values().contains(QSharedPointer<AbstractFile>(file));
    }

private:
    static QMutex m_mutex;
    static QMap<int, QSharedPointer<AbstractFile>> m_files;
    static int m_id;
};

#endif // FileHolder_H
