#ifndef EXFILE_H
#define EXFILE_H

#include "exvars.h"
#include <abstractfile.h>

namespace Excellon {

class DrillFile : public AbstractFile, public QList<Hole> {
    QMap<int, double> m_tools;
    friend class DrillParser;
    Format m_format;

public:
    DrillFile();
    ~DrillFile() {}

    FileType type() const override { return FileType::Drill; }

    double tool(int t) const;
    QMap<int, double> tools() const;

    Format format() const;
    void setFormat(const Format& value);
    void setFormatForFile(const Format& value);

protected:
    Paths merge() const override;
};
} // namespace Excellon

#endif // EXFILE_H
