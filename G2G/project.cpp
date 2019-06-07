#include "project.h"
#include "mainwindow.h"

#include <QElapsedTimer>
#include <filetree/filemodel.h>

QMap<int, QSharedPointer<AbstractFile>> Project::m_files;
bool Project::m_isModified = false;
QMutex Project::m_mutex;
QString Project::m_fileName;

Project::Project() {}

Project::~Project() {}

bool Project::save(QFile& file)
{
    try {
        QDataStream out(&file);
        out << m_files;
        m_isModified = false;
        return true;
    } catch (...) {
        qDebug() << file.errorString();
    }
    return false;
}

bool Project::open(QFile& file)
{
    try {
        QElapsedTimer t;
        t.start();
        QDataStream in(&file);
        in >> m_files;
        for (const QSharedPointer<AbstractFile>& filePtr : m_files) {
            filePtr->createGi();
            switch (filePtr->type()) {
            case FileType::Gerber:
                FileModel::addFile(static_cast<Gerber::File*>(filePtr.data()));
                break;
            case FileType::Drill:
                FileModel::addFile(static_cast<Excellon::File*>(filePtr.data()));
                break;
            case FileType::GCode:
                FileModel::addFile(static_cast<GCodeFile*>(filePtr.data()));
                break;
            }
        }
        m_isModified = false;
        qDebug() << "Project::open" << t.elapsed();
        return true;
    } catch (...) {
        qDebug() << file.errorString();
    }
    return false;
}

AbstractFile* Project::file(int id)
{
    QMutexLocker locker(&m_mutex);
    if (m_files.contains(id))
        return m_files.value(id).data();
    return nullptr;
}

void Project::deleteFile(int id)
{
    QMutexLocker locker(&m_mutex);
    if (m_files.contains(id))
        m_files.take(id);
    else
        qWarning() << "Error id" << id;
    m_isModified = true;
    MainWindow ::self->setWindowModified(true);
}

bool Project::isEmpty()
{
    QMutexLocker locker(&m_mutex);
    for (const QSharedPointer<AbstractFile>& sp : m_files) {
        if (sp.data() && (sp.data()->type() == FileType::Gerber || sp.data()->type() == FileType::Drill))
            return true;
    }
    return false;
}

int Project::size()
{
    return m_files.size();
}

QRectF Project::getSelectedBoundingRect()
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

QString Project::fileNames()
{
    QMutexLocker locker(&m_mutex);
    QString paths;
    for (const QSharedPointer<AbstractFile>& sp : m_files) {
        AbstractFile* item = sp.data();
        if (item && (item->type() == FileType::Gerber || item->type() == FileType::Drill))
            paths.append(item->name()).append('|');
    }
    return paths;
}

int Project::addFile(AbstractFile* file)
{
    QMutexLocker locker(&m_mutex);
    qDebug() << m_files << file->m_id;
    if (file->m_id == -1) {
        file->m_id = m_files.size() ? m_files.lastKey() + 1 : 0;
        m_files.insert(file->m_id, QSharedPointer<AbstractFile>(file));
    }
    m_isModified = true;
    MainWindow ::self->setWindowModified(true);
    return file->m_id;
}

bool Project::contains(AbstractFile* file)
{
    return m_files.values().contains(QSharedPointer<AbstractFile>(file));
}

QDataStream& operator<<(QDataStream& stream, const QSharedPointer<AbstractFile>& file)
{
    stream << static_cast<int>(file->type());
    file->write(stream);
    return stream;
}

QDataStream& operator>>(QDataStream& stream, QSharedPointer<AbstractFile>& file)
{
    int type;
    stream >> type;
    switch (type) {
    case FileType::Gerber:
        file = QSharedPointer<AbstractFile>(new Gerber::File);
        file->read(stream);
        break;
    case FileType::Drill:
        file = QSharedPointer<AbstractFile>(new Excellon::File);
        file->read(stream);
        break;
    case FileType::GCode:
        file = QSharedPointer<AbstractFile>(new GCodeFile);
        file->read(stream);
        break;
    }
    return stream;
}
