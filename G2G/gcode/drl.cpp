#include "drl.h"
#include "gcode.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <filetree/drillitem.h>
#include <graphicsitem.h>

Drl::Drl(QObject* parent)
    : QObject(parent)
{
}

DrlFile* Drl::parseFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return nullptr;

    qDebug() << fileName;
    m_state.reset();
    m_file = new DrlFile;
    m_file->fileName = fileName;
    QTextStream in(&file);
    QString line;
    while (in.readLineInto(&line)) {
        try {
            if (parseComment(line))
                continue;

            if (parseGCode(line))
                continue;

            if (parseMCode(line))
                continue;

            if (parseTCode(line))
                continue;

            if (parsePos(line))
                continue;

            if (parseFormat(line))
                continue;

            qDebug() << "херня какаято:" << line;

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
        m_file->itemGroup = new ItemGroup;
        for (Hole& hole : *m_file) {
            //            IntPoint center(hole.state.pos.x() * uScale, hole.state.pos.y() * uScale);
            //            double radius = hole.state.currentToolDiameter * uScale / 2.0;
            //            Path poligon(G::STEPS_PER_CIRCLE);
            //            for (int i = 0; i < G::STEPS_PER_CIRCLE; ++i) {
            //                poligon[i] = IntPoint(
            //                    (qCos(i * M_2PI / G::STEPS_PER_CIRCLE) * radius) + center.X,
            //                    (qSin(i * M_2PI / G::STEPS_PER_CIRCLE) * radius) + center.Y);
            //            }
            //            if (Area(poligon) < 0)
            //                ReversePath(poligon);

            m_file->itemGroup->append(new DrillItem(hole.state.currentToolDiameter /*{ poligon }, m_file*/));
            m_file->itemGroup->last()->setToolTip(QString("Hole %1").arg(hole.state.tCode));
            m_file->itemGroup->last()->setPos(hole.state.pos);
            m_file->itemGroup->last()->setBrush(Qt::white);
        }
    }
    return m_file;
}

bool Drl::parseComment(const QString& line)
{
    const QRegExp match(";(.*)$");
    if (match.exactMatch(line)) {
        const QRegExp matchFormat("\\D*Format:\\D*(\\d)\\.(\\d).+");
        if (matchFormat.exactMatch(match.cap(1))) {
            m_state.format.integer = matchFormat.cap(1).toInt();
            m_state.format.decimal = matchFormat.cap(2).toInt();
        }
        return true;
    }
    return false;
}

bool Drl::parseGCode(const QString& line)
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

bool Drl::parseMCode(const QString& line)
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
            m_state.format.unitMode = MILLIMETERS;
            break;
        case M72:
            m_state.mCode = M72;
            m_state.format.unitMode = INCHES;
            break;
        default:
            break;
        }
        return true;
    }
    return false;
}

bool Drl::parseTCode(const QString& line)
{
    const QRegExp match("^T([0]?[0-9]{2})(?:C([+-]?\\d*\\.?\\d+))?$");
    if (match.exactMatch(line)) {
        m_state.tCode = match.cap(1).toInt();
        if (!match.cap(2).isEmpty()) {
            const double k = m_state.format.unitMode ? 1.0 : 25.4;
            m_file->m_toolDiameter[m_state.tCode] = match.cap(2).toDouble() * k;
        } else
            m_state.currentToolDiameter = m_file->m_toolDiameter[m_state.tCode];
        return true;
    }
    return false;
}

bool Drl::parsePos(const QString& line)
{
    static double x = 0.0;
    static double y = 0.0;

    QRegExp match("(?:X([+-]?\\d*\\.?\\d+))?(?:Y([+-]?\\d*\\.?\\d+))?");
    if (match.exactMatch(line)) {

        const double k = m_state.format.unitMode ? 1.0 : 25.4;
        if (match.cap(1).contains('.') || match.cap(2).contains('.')) {
            if (match.cap(1).isEmpty())
                m_state.pos.setX(x);
            else
                m_state.pos.setX(match.cap(1).toDouble() * k);

            if (match.cap(2).isEmpty())
                m_state.pos.setY(y);
            else
                m_state.pos.setY(match.cap(2).toDouble() * k);
        } else {
            if (match.cap(1).isEmpty())
                m_state.pos.setX(x);
            else {
                double x = match.cap(1).toInt() * pow(0.1, m_state.format.decimal) * k;

                m_state.pos.setX(x);
            }

            if (match.cap(2).isEmpty())
                m_state.pos.setY(y);
            else {
                double y = match.cap(2).toInt() * pow(0.1, m_state.format.decimal) * k;
                m_state.pos.setY(y);
            }
        }
        qDebug() << match.capturedTexts() << m_state.pos;
        x = m_state.pos.x();
        y = m_state.pos.y();

        m_file->append(Hole(m_state, m_file));
        return true;
    }
    return false;
}

bool Drl::parseFormat(const QString& line)
{
    if (line.contains(QRegExp("METRIC"))) {
        m_state.format.unitMode = MILLIMETERS;
        return true;
    }
    if (line.contains(QRegExp("INCH"))) {
        m_state.format.unitMode = INCHES;
        return true;
    }
    return false;
}
