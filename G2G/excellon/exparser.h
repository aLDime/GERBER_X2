#ifndef EXPARSER_H
#define EXPARSER_H

#include "excellon.h"

namespace Excellon {
///////////////////////////////////////////////////////
/// \brief The DrillParser class
///
class DrillParser : public QObject {
    Q_OBJECT

public:
    explicit DrillParser(QObject* parent = nullptr);
    DrillFile* parseFile(const QString& fileName);
    bool isDrillFile(const QString& fileName);

signals:

private:
    bool parseComment(const QString& line);
    bool parseGCode(const QString& line);
    bool parseMCode(const QString& line);
    bool parseTCode(const QString& line);
    bool parsePos(const QString& line);
    bool parseSlot(const QString& line);
    bool parseRepeat(const QString& line);
    bool parseFormat(const QString& line);
    bool parseNumber(QString Str, double& val);

    State m_state;
    DrillFile* m_file = nullptr;
};
} // namespace Excellon

#endif // EXPARSER_H
