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
    m_state.reset();
    m_file = new DrillFile;
    m_file->setFileName(fileName);
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
            qWarning() << "exeption:" << errStr;
            //emit fileError("", QFileInfo(fileName).fileName() + "\n" + errStr);
            delete m_file;
        } catch (...) {
            qWarning() << "exeption:" << errno;
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
            m_file->itemGroup()->append(new DrillItem(hole.state.currentToolDiameter, m_file));
            m_file->itemGroup()->last()->setToolTip(QString("Tool %1, Ø%2mm(%3)").arg(hole.state.tCode).arg(hole.state.currentToolDiameter).arg(hole.state.line));
            m_file->itemGroup()->last()->setPos(hole.state.pos);
        }
        m_file->itemGroup()->setZValue(std::numeric_limits<double>::max());
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
            m_state.format.integer = matchFormat.cap(1).toInt();
            m_state.format.decimal = matchFormat.cap(2).toInt();
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
            m_state.format.unitMode = Millimeters;
            break;
        case M72:
            m_state.mCode = M72;
            m_state.format.unitMode = Inches;
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
            const double k = m_state.format.unitMode ? 1.0 : 25.4;
            m_file->m_toolDiameter[m_state.tCode] = match.cap(index + 1).toDouble() * k;
            m_state.currentToolDiameter = m_file->m_toolDiameter[m_state.tCode];
        } else
            m_state.currentToolDiameter = m_file->m_toolDiameter[m_state.tCode];
        return true;
    }
    return false;
}

bool DrillParser::parsePos(const QString& line)
{
    static double x = 0.0;
    static double y = 0.0;

    QRegExp match("^(?:G(\\d+))?"
                  "(?:X([+-]?\\d*\\.?\\d+))?"
                  "(?:Y([+-]?\\d*\\.?\\d+))?"
                  ".*$");
    if (match.exactMatch(line)) {
        QStringList capturedTexts(match.capturedTexts());
        qDebug() << capturedTexts;
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
        qDebug() << match.capturedTexts();
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
        qDebug() << match.capturedTexts();
        switch (unitMode.indexOf(match.cap(1))) {
        case Inches:
            m_state.format.unitMode = Inches;
            break;
        case Millimeters:
            m_state.format.unitMode = Millimeters;
            break;
        default:
            break;
        }
        switch (zeroMode.indexOf(match.cap(2))) {
        case LeadingZeros:
            m_state.format.zeroMode = LeadingZeros;
            break;
        case TrailingZeros:
            m_state.format.zeroMode = TrailingZeros;
            break;
        default:
            break;
        }
        return true;
    }
    static const QRegExp match2("^(FMAT).*(2)?$");
    if (match2.exactMatch(line)) {
        qDebug() << match2.capturedTexts();
        m_state.format.unitMode = Inches;
        m_state.format.zeroMode = LeadingZeros;
        return true;
    }

    return false;
}

bool DrillParser::parseNumber(QString Str, double& val, int integer, int decimal)
{
    bool flag = false;
    int sign = 1;
    if (!Str.isEmpty()) {
        if (Str.contains('.')) {
            val = Str.toDouble();
        } else {
            if (!decimal)
                decimal = m_state.format.decimal;
            if (!integer)
                integer = m_state.format.integer;

            if (Str.startsWith('+')) {
                Str.remove(0, 1);
                sign = 1;
            } else if (Str.startsWith('-')) {
                Str.remove(0, 1);
                sign = -1;
            }
            if (Str.length() < integer + decimal) {
                switch (m_state.format.zeroMode) {
                case LeadingZeros:
                    Str = Str + QString(integer + decimal - Str.length(), '0');
                    break;
                case TrailingZeros:
                    Str = QString(integer + decimal - Str.length(), '0') + Str;
                    break;
                }
            }
            val = Str.toDouble() * pow(10.0, -decimal) * sign;
        }
        if (m_state.format.unitMode == Inches)
            val *= 25.4;
        return true;
    }
    return flag;
}
