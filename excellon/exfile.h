#ifndef EXFILE_H
#define EXFILE_H

#include "exvars.h"
#include <abstractfile.h>

namespace Excellon {

class File : public AbstractFile, public QList<Hole> {
    QMap<int, double> m_tools;
    friend class Parser;
    Format m_format;

public:
    File();
    ~File();

    FileType type() const override { return FileType::Drill; }

    double tool(int t) const;
    QMap<int, double> tools() const;

    Format format() const;
    void setFormat(const Format& value);
    void setFormatForFile(const Format& value);

    void saveFormat();
    void restoreFormat();
    ItemGroup* itemGroup() const override { return m_itemGroup.data(); }

protected:
    Paths merge() const override;

    // AbstractFile interface
public:
    virtual void write(QDataStream& stream) const override;
    virtual void read(QDataStream& stream) override;
    virtual void createGi() override;
};
} // namespace Excellon

#endif // EXFILE_H
