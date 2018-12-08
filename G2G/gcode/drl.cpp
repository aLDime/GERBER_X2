#include "drl.h"
#include "gcode.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <filetree/drillnode.h>
#include <gi/drillitem.h>
#include <limits>

DrillParser::DrillParser(QObject* parent)
    : QObject(parent)
{
}

DrillFile* DrillParser::parseFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return nullptr;
    qDebug() << fileName;

    m_file = new DrillFile;
    m_file->setFileName(fileName);
    m_state.reset(&m_file->m_format);

    QTextStream in(&file);
    QString line;
    while (in.readLineInto(&line)) {
        m_file->lines().append(line);
        ++m_state.line;
        try {
            if (parseComment(line))
                continue;

            if (parseFormat(line))
                continue;

            if (parseGCode(line))
                continue;

            if (parseMCode(line))
                continue;

            if (parseTCode(line))
                continue;

            if (parseRepeat(line))
                continue;

            if (parsePos(line))
                continue;

            qWarning() << "херня какаято:" << line;

        } catch (const QString& errStr) {
            qWarning() << "exeption Q:" << errStr;
            //emit fileError("", QFileInfo(fileName).fileName() + "\n" + errStr);
            delete m_file;
        } catch (...) {
            qWarning() << "exeption S:" << errno;
            //emit fileError("", QFileInfo(fileName).fileName() + "\n" + "Unknown Error!");
            delete m_file;
        }
    }
    if (m_file->isEmpty()) {
        delete m_file;
        m_file = nullptr;
    } else {
        m_file->setItemGroup(new ItemGroup);
        for (Hole& hole : *m_file) {
            DrillItem* item = new DrillItem(&hole);
            hole.item = item;
            m_file->itemGroup()->append(item);
        }
    }
    return m_file;
}

bool DrillParser::isDrillFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;
    QTextStream in(&file);
    QString line;
    const QRegExp match("^T([0]?[0-9]{1})[FSC]((\\d*\\.?\\d+))?.*$");
    while (in.readLineInto(&line)) {
        if (match.exactMatch(line)) {
            //qDebug() << match.capturedTexts();
            return true;
        }
    }
    return false;
}

bool DrillParser::parseComment(const QString& line)
{
    const QRegExp match("^;(.*)$");
    if (match.exactMatch(line)) {
        const QRegExp matchFormat(".*FORMAT.*([0-9]).([0-9]).*", Qt::CaseInsensitive);
        if (matchFormat.exactMatch(match.cap(1))) {
            //qDebug() << matchFormat.capturedTexts();
            m_file->m_format.integer = matchFormat.cap(1).toInt();
            m_file->m_format.decimal = matchFormat.cap(2).toInt();
        }
        return true;
    }
    return false;
}

bool DrillParser::parseGCode(const QString& line)
{
    const QRegExp match("^G([0]?[0-9]{2})$");
    if (match.exactMatch(line)) {
        switch (match.cap(1).toInt()) {
        case G05:
            m_state.gCode = G05;
            break;
        case G90:
            m_state.gCode = G90;
            break;
        default:
            break;
        }
        return true;
    }
    return false;
}

bool DrillParser::parseMCode(const QString& line)
{
    const QRegExp match("^M([0]?[0-9]{2})$");
    if (match.exactMatch(line)) {
        switch (match.cap(1).toInt()) {
        case M30:
            m_state.mCode = M30;
            break;
        case M48:
            m_state.mCode = M48;
            break;
        case M71:
            m_state.mCode = M71;
            m_file->m_format.unitMode = Millimeters;
            break;
        case M72:
            m_state.mCode = M72;
            m_file->m_format.unitMode = Inches;
            break;
        case M95:
            m_state.mCode = M95;
            break;
        default:
            break;
        }
        return true;
    }
    if (line == "%" && m_state.mCode == M48) {
        m_state.mCode = M95;
        return true;
    }
    return false;
}

bool DrillParser::parseTCode(const QString& line)
{
    const QRegExp match("^T([0-9]{1,2})"
                        "(?:([CFS])(\\d*\\.?\\d+))?"
                        "(?:([CFS])(\\d*\\.?\\d+))?"
                        "(?:([CFS])(\\d*\\.?\\d+))?"
                        ".*$");
    //const QRegExp match("^T([0]?[0-9]{1})(?:C(\\d*\\.?\\d+))?.*$");
    if (match.exactMatch(line)) {
        const QStringList capturedTexts(match.capturedTexts());
        const int index = capturedTexts.indexOf("C");
        //        qDebug() << capturedTexts << index;
        m_state.tCode = match.cap(1).toInt();
        if (index > 0) {
            //            const double k = m_file->format.unitMode ? 1.0 : 25.4;
            m_file->m_tools[m_state.tCode] = match.cap(index + 1).toDouble() /* * k*/;
            //            m_state.currentToolDiameter = m_file->m_tools[m_state.tCode];
        } /*else
            m_state.currentToolDiameter = m_file->m_tools[m_state.tCode];*/
        return true;
    }
    return false;
}

bool DrillParser::parsePos(const QString& line)
{
    QRegExp match("^(?:G(\\d+))?"
                  "(?:X([+-]?\\d*\\.?\\d+))?"
                  "(?:Y([+-]?\\d*\\.?\\d+))?"
                  ".*$");
    if (match.exactMatch(line)) {

        if (!match.cap(2).isEmpty())
            m_state.rawPos[0] = match.cap(2);
        if (!match.cap(3).isEmpty())
            m_state.rawPos[1] = match.cap(3);

        parseNumber(match.cap(2), m_state.pos.rx());
        parseNumber(match.cap(3), m_state.pos.ry());
        m_file->append(Hole(m_state, m_file));
        return true;
    }
    return false;
}

bool DrillParser::parseRepeat(const QString& line)
{

    QRegExp match("^R(\\d+)"
                  "(?:X([+-]?\\d*\\.?\\d+))?"
                  "(?:Y([+-]?\\d*\\.?\\d+))?"
                  "$");
    if (match.exactMatch(line)) {
        int count = match.cap(1).toInt();
        QPointF p;
        parseNumber(match.cap(2), p.rx());
        parseNumber(match.cap(3), p.ry());
        for (int i = 0; i < count; ++i) {
            m_state.pos += p;
            m_file->append(Hole(m_state, m_file));
        }
        return true;
    }
    return false;
}

bool DrillParser::parseFormat(const QString& line)
{
    static const QVector<QString> unitMode({ QStringLiteral("INCH"), QStringLiteral("METRIC") });
    static const QVector<QString> zeroMode({ QStringLiteral("LZ"), QStringLiteral("TZ") });
    static const QRegExp match("^(METRIC|INCH).*(LZ|TZ)?$");
    if (match.exactMatch(line)) {
        switch (unitMode.indexOf(match.cap(1))) {
        case Inches:
            m_file->m_format.unitMode = Inches;
            break;
        case Millimeters:
            m_file->m_format.unitMode = Millimeters;
            break;
        default:
            break;
        }
        switch (zeroMode.indexOf(match.cap(2))) {
        case LeadingZeros:
            m_file->m_format.zeroMode = LeadingZeros;
            break;
        case TrailingZeros:
            m_file->m_format.zeroMode = TrailingZeros;
            break;
        default:
            break;
        }
        return true;
    }
    static const QRegExp match2("^(FMAT).*(2)?$");
    if (match2.exactMatch(line)) {
        m_file->m_format.unitMode = Inches;
        m_file->m_format.zeroMode = LeadingZeros;
        return true;
    }

    return false;
}

bool DrillParser::parseNumber(QString Str, double& val)
{
    bool flag = false;
    int sign = 1;
    if (!Str.isEmpty()) {
        if (Str.contains('.')) {
            val = Str.toDouble();
        } else {

            if (Str.startsWith('+')) {
                Str.remove(0, 1);
                sign = 1;
            } else if (Str.startsWith('-')) {
                Str.remove(0, 1);
                sign = -1;
            }
            if (Str.length() < m_file->m_format.integer + m_file->m_format.decimal) {
                switch (m_file->m_format.zeroMode) {
                case LeadingZeros:
                    Str = Str + QString(m_file->m_format.integer + m_file->m_format.decimal - Str.length(), '0');
                    break;
                case TrailingZeros:
                    Str = QString(m_file->m_format.integer + m_file->m_format.decimal - Str.length(), '0') + Str;
                    break;
                }
            }
            val = Str.toDouble() * pow(10.0, -m_file->m_format.decimal) * sign;
        }
        if (m_file->m_format.unitMode == Inches)
            val *= 25.4;
        return true;
    }
    return flag;
}

Format DrillFile::format() const
{
    return m_format;
}

void DrillFile::setFormat(const Format& value)
{
    m_format.zeroMode = value.zeroMode;
    m_format.unitMode = value.unitMode;
    m_format.decimal = value.decimal;
    m_format.integer = value.integer;
    m_format.offsetPos = value.offsetPos;
    for (Hole& hole : *this) {
        hole.state.updatePos();
        hole.item->updateHole();
    }
}

double DrillFile::tool(int t) const
{
    double tool = 0.0;
    if (m_tools.contains(t)) {
        tool = m_tools[t];
        if (m_format.unitMode == Inches)
            tool *= 25.4;
    }
    return tool;
}

QMap<int, double> DrillFile::tools() const
{
    QMap<int, double> tools(m_tools);
    QMap<int, double>::iterator toolIt;
    if (m_format.unitMode == Inches)
        for (toolIt = tools.begin(); toolIt != tools.end(); ++toolIt)
            toolIt.value() *= 25.4;
    return tools;
}

///////////////////////////////////////////////////////
/// \brief State::reset
/// \param f
///
void State::reset(Format* f)
{
    format = f;
    if (format) {
        format->unitMode = Millimeters;
        format->decimal = 4;
        format->integer = 3;
    }
    rawPos[0].clear();
    rawPos[1].clear();
    gCode = G00;
    mCode = M00;
    tCode = 0;
    pos = QPointF();
    path.clear();
    line = 0;
}
///////////////////////////////////////////////////////
/// \brief State::updatePos
///
void State::updatePos()
{
    pos = QPointF(parseNumber(rawPos[0]), parseNumber(rawPos[1]));
}
///////////////////////////////////////////////////////
/// \brief State::parseNumber
/// \param Str
/// \return
///
double State::parseNumber(QString Str)
{
    double val = 0.0;
    int sign = 1;
    if (!Str.isEmpty()) {
        if (Str.contains('.')) {
            val = Str.toDouble();
        } else {

            if (Str.startsWith('+')) {
                Str.remove(0, 1);
                sign = 1;
            } else if (Str.startsWith('-')) {
                Str.remove(0, 1);
                sign = -1;
            }
            if (Str.length() < format->integer + format->decimal) {
                switch (format->zeroMode) {
                case LeadingZeros:
                    Str = Str + QString(format->integer + format->decimal - Str.length(), '0');
                    break;
                case TrailingZeros:
                    Str = QString(format->integer + format->decimal - Str.length(), '0') + Str;
                    break;
                }
            }
            val = Str.toDouble() * pow(10.0, -format->decimal) * sign;
        }
        if (format->unitMode == Inches)
            val *= 25.4;
    }
    return val;
}
///////////////////////////////////////////////////////
/// \brief State::currentToolDiameter
/// \return
///
double State::currentToolDiameter() const
{
    return format->file->tool(tCode);
}
