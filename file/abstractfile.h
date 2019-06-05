#ifndef GAbstractFile_H
#define GAbstractFile_H

#include <QDateTime>
#include <QFileInfo>
#include <myclipper.h>

#include <gi/itemgroup.h>

using namespace ClipperLib;
enum class FileType {
    Gerber,
    Drill,
    GCode,
};

enum Side {
    NullSide = -1,
    Top,
    Bottom
};

//template <typename T = int>
class AbstractFile { // : public QList<T> {
public:
    AbstractFile();
    ~AbstractFile();

    QString shortName() const;
    QString name() const;
    void setFileName(const QString& name);

    virtual ItemGroup* itemGroup() const;
    void setItemGroup(ItemGroup* itemGroup);

    Paths mergedPaths() const;
    Pathss groupedPaths() const;

    QList<QString>& lines();

    enum Group {
        CopperGroup,
        CutoffGroup,
    };

    virtual FileType type() const = 0;
    virtual void write() const = 0;
    virtual void read() = 0;
    virtual void createGi() = 0;

    Side side() const;
    void setSide(Side side);

    QColor color() const;
    void setColor(const QColor& color);

    int id() const;

    friend QDataStream& operator<<(QDataStream& stream, const AbstractFile* af)
    {
        stream << af->m_id;
        stream << af->m_lines;
        stream << af->m_name;
        stream << af->m_mergedPaths;
        stream << af->m_groupedPaths;
        stream << af->m_side;
        stream << af->m_color;
        stream << af->m_date;
        return stream;
    }

    friend QDataStream& operator>>(QDataStream& stream, AbstractFile* af)
    {
        int tmp;
        stream >> af->m_id;
        stream >> af->m_lines;
        stream >> af->m_name;
        stream >> af->m_mergedPaths;
        stream >> af->m_groupedPaths;
        stream >> tmp;
        af->m_side = static_cast<Side>(tmp);
        stream >> af->m_color;
        stream >> af->m_date;
        return stream;
    }

protected:
    int m_id;
    virtual Paths merge() const = 0;

    QSharedPointer<ItemGroup> m_itemGroup;
    QList<QString> m_lines;
    QString m_name;
    mutable Paths m_mergedPaths;
    Pathss m_groupedPaths;

    Side m_side = Top;
    QColor m_color;
    QDateTime m_date;
};

//Q_DECLARE_METATYPE(AbstractFile)

#endif // GAbstractFile_H
