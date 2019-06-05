#include "project.h"

QMap<int, QSharedPointer<AbstractFile>> Project::m_files;
int Project::m_id = 0;
QMutex Project::m_mutex;

Project::Project(const QString& fileName)
    : fileName(fileName)
{

}

Project::~Project()
{
}

AbstractFile* Project::file(int id)
{
    QMutexLocker locker(&m_mutex);
    return m_files.value(id).data();
}

void Project::deleteFile(int id)
{
    QMutexLocker locker(&m_mutex);
    if (m_files.contains(id))
        m_files.take(id);
    else
        qWarning() << "Error id";
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

bool Project::contains(AbstractFile* file)
{
    return m_files.values().contains(QSharedPointer<AbstractFile>(file));
}
