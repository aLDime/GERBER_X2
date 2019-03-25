#ifndef GAbstractFile_H
#define GAbstractFile_H

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

    QString shortFileName() const;
    QString fileName() const;
    void setFileName(const QString& fileName);

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

    Side side() const;
    void setSide(Side side);

    QColor color() const;
    void setColor(const QColor& color);

protected:
    virtual Paths merge() const = 0;

    QSharedPointer<ItemGroup> m_itemGroup;
    QList<QString> m_lines;
    QString m_fileName;
    mutable Paths m_mergedPaths;
    Pathss m_groupedPaths;

    Side m_side = Top;
    QColor m_color;
};

//Q_DECLARE_METATYPE(AbstractFile)

#endif // GAbstractFile_H
