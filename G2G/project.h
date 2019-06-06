#ifndef PROJECT_H
#define PROJECT_H

#include <QMap>
#include <QMutex>
#include <QObject>
#include <exfile.h>
#include <gbrfile.h>
#include <gcode/gcode.h>
#include <myclipper.h>

using namespace ClipperLib;

class Project {

public:
    explicit Project(const QString& fileName = QString());
    ~Project();

    static bool fl;

    bool save(const QString& fileName = QString());
    bool open(const QString& fileName);

    static AbstractFile* file(int id);
    static void deleteFile(int id);
    static bool isEmpty();
    static int size();

    //    static Paths getPaths()
    //    {
    //        QMutexLocker locker(&m_mutex);
    //        Paths paths;
    //        for (const QSharedPointer<AbstractFile>& sp : m_files) {
    //            AbstractFile* file = sp.data();
    //            if (file
    //                    && file->itemGroup()->isVisible()
    //                    && (file->type() == FileType::Gerber || file->type() == FileType::Drill))
    //                for (Paths& p : file->groupedPaths())
    //                    paths.append(p);
    //        }
    //        return paths;
    //    }
    //    static Paths getSelectedPaths()
    //    {
    //        QMutexLocker locker(&m_mutex);
    //        Paths paths;
    //        for (const QSharedPointer<AbstractFile>& sp : m_files) {
    //            AbstractFile* file = sp.data();
    //            if (file
    //                    && file->itemGroup()->isVisible()
    //                    && (file->type() == FileType::Gerber || file->type() == FileType::Drill))
    //                for (GraphicsItem* item : *file->itemGroup())
    //                    if (item->isSelected())
    //                        paths.append(item->paths());
    //        }
    //        return paths;
    //    }

    static QRectF getSelectedBoundingRect();
    static QString fileNames();

    template <typename T>
    static T* file(int id)
    {
        QMutexLocker locker(&m_mutex);
        return static_cast<T*>(m_files.value(id).data());
    }

    static int addFile(AbstractFile* file);

    template <typename T>
    static bool replaceFile(int id, T* file)
    {
        QMutexLocker locker(&m_mutex);
        if (m_files.contains(id)) {
            m_files[id] = QSharedPointer<AbstractFile>(file);
            return true;
        }
        return false;
    }

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

    template <typename T>
    static QVector<T*> count()
    {
        QMutexLocker locker(&m_mutex);
        int count;
        for (const QSharedPointer<AbstractFile>& sp : m_files) {
            if (dynamic_cast<T*>(sp.data()))
                ++count;
        }
        return count;
    }

    static bool contains(AbstractFile* file);

private:
    static QMutex m_mutex;
    static QMap<int, QSharedPointer<AbstractFile>> m_files;
    QString m_fileName;
};

#endif // PROJECT_H
