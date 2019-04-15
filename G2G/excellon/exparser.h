#ifndef EXPARSER_H
#define EXPARSER_H

#include "exvars.h"
#include <QObject>

namespace Excellon {

class File;

class Parser : public QObject {
    Q_OBJECT

public:
    explicit Parser(QObject* parent = nullptr);
    File* parseFile(const QString& fileName);
    bool isDrillFile(const QString& fileName);
    static double parseNumber(QString Str, const State& state);

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
    File* m_file = nullptr;
};
} // namespace Excellon

#endif // EXPARSER_H
