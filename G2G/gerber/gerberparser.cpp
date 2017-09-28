#include "gerberparser.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QTextStream>
#include <QThread>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

int id1 = qRegisterMetaType<GERBER_FILE*>("GERBER_FILE*");

const QList<QString> slApertureType(QString("C|R|O|P|M").split("|"));
const QList<QString> slAttributeType(QString("TF|TA|TO|TD").split("|"));
const QList<QString> slImagePolarity(QString("POS|NEG").split("|"));
const QList<QString> slLevelPolarity(QString("D|C").split("|"));
const QList<QString> slUnitType(QString("IN|MM").split("|"));

const QRegExp reAd("^%ADD(\\d\\d+)([a-zA-Z_$\\.][a-zA-Z0-9_$\\.\\-]*)(?:,(.*))?\\*%$");
const QRegExp reAmBegin("^%AM([^\\*]+)\\*([^%]+)?(%)?$");
const QRegExp reAttributes("^%(TF|TA|TO|TD)(.*)\\*%$");
const QRegExp reCirc("^(?:G0?([23]))?[X]?([\\+-]?\\d+)*[Y]?([\\+-]?\\d+)*[I]?([\\+-]?\\d+)*[J]?([\\+-]?\\d+)*[^D]*(?:D0?([12]))?\\*$");
const QRegExp reComment("^G0?4(.*)$");
const QRegExp reDCode("^D0?([123])\\*$");
const QRegExp reEof1("^M[0]?[0123]\\*");
const QRegExp reEof2("^D0?2M0?[02]\\*");
const QRegExp reFormat("^%FS([LT]?)([AI]?)X(\\d)(\\d)Y(\\d)(\\d)\\*%$");
const QRegExp reGCode("^G([0]?[0-9]{2})\\*$");
const QRegExp reInterp("^(?:G0?([123]))\\*");
const QRegExp reLin("^(?:G0?(1))?(?=.*X([\\+-]?\\d+))?(?=.*Y([\\+-]?\\d+))?[XY]*[^DIJ]*(?:D0?([123]))?\\*$");
const QRegExp reLpol("^%LP([DC])\\*%$");
const QRegExp rePol("^%IP(POS|NEG)\\*%$");
const QRegExp reStepAndRepeat("^%SR(?:X(\\d+))?(?:Y(\\d+))?(?:I(\\d+\\.?\\d*))?(?:J(\\d+\\.?\\d*))?\\*%$");
const QRegExp reTool("^(?:G54)?D(\\d\\d+)\\*$");
const QRegExp reUnitMode("^%MO(IN|MM)\\*%$");

QMutex mutex;

GerberParser::GerberParser(QObject* parent)
    : QObject(parent)
{
}

GERBER_FILE* GerberParser::ParseFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return nullptr;
    }
    QTextStream in(&file);
    return ParseLines(in.readAll(), fileName);
}

GERBER_FILE* GerberParser::ParseLines(const QString& gerberLines, const QString& fileName)
{
    mutex.lock();

    QRegExp match("FS([LT]?)([AI]?)X(\\d)(\\d)Y(\\d)(\\d)\\*");
    if (match.indexIn(gerberLines) == -1) {
        mutex.unlock();
        return nullptr;
    }

    if (fileName.isEmpty())
        Reset(QDateTime::currentDateTime().toString("dd.MM.yyyy-hh:mm:ss.zzz"));
    else
        Reset(fileName);

    gerbFile->lines = Format(gerberLines);

    for (QString& gerberLine : gerbFile->lines) {
        try {
            gerbLines.push_back(gerberLine);
            ++state.lineNum;
            //qWarning() << QString("Line Num %1: '%2'").arg(state.lineNum).arg(gLine);
            if (ParseGCode(gerberLine)) {
                continue;
            }
            if (ParseAttributes(gerberLine)) {
                continue;
            }
            if (ParseStepAndRepeat(gerberLine)) {
                continue;
            }
            if (ParseImagePolarity(gerberLine)) {
                continue;
            }
            if (ParseApertureMacros(gerberLine)) {
                continue;
            }
            if (ParseEndOfFile(gerberLine)) {
                continue;
            }
            if (ParseOperationDCode(gerberLine)) {
                continue;
            }
            // Aperture definitions %ADD...
            if (ParseAperture(gerberLine)) {
                continue;
            }
            // D12*
            if (ParseToolAperture(gerberLine)) {
                continue;
            }
            // Polarity change
            // Example: %LPD*% or %LPC*%
            // If polarity changes, creates geometry from current
            // buffer, then adds or subtracts accordingly.
            if (ParseLevelPolarity(gerberLine)) {
                continue;
            }
            // Number format
            // Example: %FSLAX24Y24*%
            // TODO: This is ignoring most of the format. Implement the rest.
            if (ParseFormat(gerberLine)) {
                continue;
            }
            // Mode (IN/MM)
            // Example: %MOIN*%
            if (ParseUnitMode(gerberLine)) {
                continue;
            }
            // G01 - Linear interpolation plus flashes
            // Operation code (D0x) missing is deprecated... oh well I will support it.
            // REGEX: r"^(?:G0?(1))?(?:X(-?\d+))?(?:Y(-?\d+))?(?:D0([123]))?\*$"
            if (ParseLineInterpolation(gerberLine)) {
                continue;
            }
            // G02/3 - Circular interpolation
            // 2-clockwise, 3-counterclockwise
            if (ParseCircularInterpolation(gerberLine)) {
                continue;
            }
            // Line did not match any pattern. Warn user.
            // qWarning() << QString("Line ignored (%1): '%2'").arg(state.lineNum).arg(gerberLine);
        }
        catch (const QString& errStr) {
            qWarning() << "exeption:" << errStr;
        }
        catch (...) {
            qWarning() << "exeption:" << errno;
        }
    }

    if (gerbFile->isEmpty())
        delete gerbFile;
    else
        emit resultReady(gerbFile);

    gerbLines.clear();
    apertureMacro.clear();
    curPath.clear();

    mutex.unlock();
    return gerbFile;
}

QList<QString> GerberParser::Format(QString data)
{
    QList<QString> gerberLines;

    enum SATE {
        PARAM,
        MACRO,
        DATA,
    };

    SATE state = DATA;
    QString lastLine;

    auto gerberLinesAppend = [&gerberLines, &lastLine](SATE& state, const QString& val) -> void {
        switch (state) {
        case MACRO:
            lastLine.push_back(val);
            if (lastLine.endsWith('%')) {
                gerberLines << lastLine;
                state = DATA;
            }
            break;
        case PARAM:
            lastLine.push_back(val);
            if (lastLine.endsWith('%')) {
                for (QString& tmpline : lastLine.remove('%').split('*')) {
                    if (!tmpline.isEmpty()) {
                        gerberLines << ('%' + tmpline + "*%");
                    }
                }
                state = DATA;
            }
            break;
        case DATA:
            break;
        }
    };

    auto lastLineClose = [&gerberLines](SATE state, QString& val) -> void {
        switch (state) {
        case MACRO:
            if (!val.endsWith('%'))
                val.push_back('%');
            if (!val.endsWith("*%"))
                val.insert(val.length() - 2, '*');
            gerberLines << val;
            break;
        case PARAM:
            for (QString& tmpline : val.remove('%').split('*')) {
                if (!tmpline.isEmpty()) {
                    gerberLines << ('%' + tmpline + "*%");
                }
            }
            break;
        case DATA:
            break;
        }
        val.clear();
    };

    auto dataClose = [&gerberLines](const QString& val) -> void {
        if (val.count('*') > 1) {
            for (QString& tmpline : val.split('*')) {
                if (!tmpline.isEmpty()) {
                    gerberLines << (tmpline + '*');
                }
            }
        }
        else {
            gerberLines << val;
        }
    };

    for (QString& line : data.replace('\r', '\n').replace("\n\n", "\n").replace('\t', ' ').split('\n')) {
        line = line.trimmed();
        if (line.isEmpty()) {
            continue;
        }
        if (line.startsWith('%') && line.endsWith('%') && line.size() > 1) {
            lastLineClose(state, lastLine);
            if (line.startsWith("%AM")) {
                lastLineClose(MACRO, line);
            }
            else {
                lastLineClose(PARAM, line);
            }
            state = DATA;
            continue;
        }
        else if (line.startsWith("%AM")) {
            lastLineClose(state, lastLine);
            state = MACRO;
            lastLine = line;
            continue;
        }
        else if (line.startsWith('%')) {
            lastLineClose(state, lastLine);
            state = PARAM;
            lastLine = line;
            continue;
        }
        else if (line.endsWith('*') && line.length() > 1) {
            switch (state) {
            case MACRO:
            case PARAM:
                gerberLinesAppend(state, line);
                continue;
            case DATA:
                dataClose(line);
                continue;
            }
        }
        else {
            switch (state) {
            case MACRO:
            case PARAM:
                gerberLinesAppend(state, line);
                continue;
            case DATA:
                qDebug() << "Хрен его знает:" << line;
                continue;
            }
        }
    }
    return gerberLines;
}
#include <math.h>
double GerberParser::ArcAngle(double start, double stop)
{
    if (state.interpolation == COUNTERCLOCKWISE_CIRCULAR && stop <= start) {
        stop += 2.0 * M_PI;
    }
    if (state.interpolation == CLOCKWISE_CIRCULAR && stop >= start) {
        stop -= 2.0 * M_PI;
    }
    return qAbs(stop - start);
}

double GerberParser::Double(const QString& Str, bool scale, bool inchControl)
{
    bool ok;
    double d = Str.toDouble(&ok);
    if (state.format.unitMode == INCHES && inchControl)
        d *= 25.4;
    if (scale)
        d *= uScale;
    return d;
}

bool GerberParser::ParseNumber(QString Str, cInt& val, int integer, int decimal)
{
    bool flag = false;
    int sign = 1;
    if (!Str.isEmpty()) {
        if (!decimal) {
            decimal = state.format.xDecimal;
        }
        if (!integer) {
            integer = state.format.xInteger;
        }
        if (Str.indexOf("+") == 0) {
            Str.remove(0, 1);
            sign = 1;
        }
        if (Str.indexOf("-") == 0) {
            Str.remove(0, 1);
            sign = -1;
        }
        if (Str.count('.')) {
            Str.setNum(Str.split('.').first().toInt() + ("0." + Str.split('.').last()).toDouble());
        }
        while (Str.length() < integer + decimal) {
            switch (state.format.zeroOmisMode) {
            case OMIT_LEADING_ZEROS:
                Str = QString(integer + decimal - Str.length(), '0') + Str;
                //Str = "0" + Str;
                break;
            case OMIT_TRAILING_ZEROS:
                Str += QString(integer + decimal - Str.length(), '0');
                //Str += "0";
                break;
            }
        }
        val = Double(Str, true) * pow(10.0, -decimal) * sign;
        return true;
    }
    return flag;
}

void GerberParser::ClosePath()
{
    if (curPath.size() < 2) {
        curPath.clear();
        curPath.push_back(state.curPos);
        return;
    }
    switch (state.region) {
    case ON:
        state.type = REGION;
        gerbFile->append(GERBER_ITEM(state, CreatePolygon(), gerbFile, gerbLines, curPath));
        break;
    case OFF:
        state.type = LINE;
        gerbFile->append(GERBER_ITEM(state, CreateLine(), gerbFile, gerbLines, curPath));
        break;
    }
    ClearStep();
}

void GerberParser::CreateFlash()
{
    state.type = APERTURE;
    Paths paths(gerbFile->apertures[state.curAperture].Draw(state));
    if (gerbFile->apertures[state.curAperture].HoleDiameter() > 0) {
        paths.push_back(gerbFile->apertures[state.curAperture].GetHole(state));
    }
    gerbFile->append(GERBER_ITEM(state, paths, gerbFile, gerbLines));
    ClearStep();
}

void GerberParser::Reset(const QString& fileName)
{
    gerbLines.clear();
    apertureMacro.clear();
    curPath.clear();
    gerbFile = new GERBER_FILE;
    gerbFile->fileName = QFileInfo(fileName).fileName();
    state = STATE();
}

void GerberParser::ClearStep()
{
    gerbLines.clear();
    curPath.clear();
    curPath.push_back(state.curPos);
}

IntPoint GerberParser::ParsePosition(const QString& xyStr)
{
    QRegExp match("(?:G[01]{1,2})?(?:X([+-]?\\d*\\.?\\d+))?(?:Y([+-]?\\d*\\.?\\d+))?");
    if (match.indexIn(xyStr) > -1) {
        cInt tmp = 0;
        if (ParseNumber(match.cap(1), tmp, state.format.xInteger, state.format.xDecimal)) {
            state.format.coordValueNotation == ABSOLUTE_NOTATION ? state.curPos.X = tmp : state.curPos.X += tmp;
        }
        tmp = 0;
        if (ParseNumber(match.cap(2), tmp, state.format.yInteger, state.format.yDecimal)) {
            state.format.coordValueNotation == ABSOLUTE_NOTATION ? state.curPos.Y = tmp : state.curPos.Y += tmp;
        }
    }
    if (2.0e-310 > state.curPos.X && state.curPos.X > 0.0) {
        throw QString("line num %1: '%2', error value.").arg(state.lineNum).arg(gerbLines.last());
    }
    if (2.0e-310 > state.curPos.Y && state.curPos.Y > 0.0) {
        throw QString("line num %1: '%2', error value.").arg(state.lineNum).arg(gerbLines.last());
    }
    return state.curPos;
}

Path GerberParser::Arc(const IntPoint& center, double radius, double start, double stop)
{
    const double da_sign[4] = { 0, 0, -1.0, +1.0 };
    Path points;
    if (1) {
        if (state.interpolation == CLOCKWISE_CIRCULAR && stop >= start)
            stop -= 2.0 * M_PI;
        else if (state.interpolation == COUNTERCLOCKWISE_CIRCULAR && stop <= start)
            stop += 2.0 * M_PI;

        double angle = qAbs(stop - start);
        double steps = qMax((int)ceil(angle / (2.0 * M_PI) * STEPS_PER_CIRCLE), 2);
        double delta_angle = da_sign[state.interpolation] * angle * 1.0 / steps;
        for (int i = 0; i < steps; i++) {
            double theta = start + delta_angle * (i + 1);
            points.push_back(IntPoint(center.X + radius * cos(theta), center.Y + radius * sin(theta)));
        }
    }
    else {
        if (state.interpolation == CLOCKWISE_CIRCULAR && stop >= start)
            stop -= 2.0 * M_PI;
        else if (state.interpolation == COUNTERCLOCKWISE_CIRCULAR && stop <= start)
            stop += 2.0 * M_PI;

        if (state.interpolation == CLOCKWISE_CIRCULAR) {
            double angle = (2.0 * M_PI);
            while (start < angle || qFuzzyCompare(start, angle))
                angle -= (2.0 * M_PI) / STEPS_PER_CIRCLE;
            for (int i = 0; i < STEPS_PER_CIRCLE; i++) {
                if (angle > stop || qFuzzyCompare(stop, angle))
                    points.push_back(IntPoint(center.X + radius * cos(angle), center.Y + radius * sin(angle)));
                else
                    break;
                angle -= (2.0 * M_PI) / STEPS_PER_CIRCLE;
            }
        }
        else if (state.interpolation == COUNTERCLOCKWISE_CIRCULAR) {
            double angle = -(2.0 * M_PI);
            while (start >= angle || qFuzzyCompare(start, angle))
                angle += (2.0 * M_PI) / STEPS_PER_CIRCLE;
            for (int i = 0; i < STEPS_PER_CIRCLE; i++) {
                if (angle <= stop || qFuzzyCompare(stop, angle))
                    points.push_back(IntPoint(center.X + radius * cos(angle), center.Y + radius * sin(angle)));
                else
                    break;
                angle += (2.0 * M_PI) / STEPS_PER_CIRCLE;
            }
        }
    }
    return points;
}

Path GerberParser::Arc2(IntPoint p1, IntPoint p2, IntPoint center)
{
    double radius = sqrt(pow((center.X - p1.X), 2) + pow((center.Y - p1.Y), 2));
    double start = atan2(p1.Y - center.Y, p1.X - center.X);
    double stop = atan2(p2.Y - center.Y, p2.X - center.X);
    return Arc(center, radius, start, stop);
}

Paths GerberParser::CreateLine()
{

    Paths solution;
    if (0) {
        Path pattern = gerbFile->apertures[state.lstAperture].Draw(state)[0];
        if (Area(pattern) < 0) {
            ReversePath(pattern);
        }
        //pattern.push_back(pattern[0]);
        MinkowskiSum(pattern, curPath, solution, false);
        if (state.imgPolarity == NEGATIVE) {
            ReversePaths(solution);
        }
    }
    else {
        double size = gerbFile->apertures[state.lstAperture].Size() * uScale * 0.5;
        if (qFuzzyIsNull(size))
            size = 0.01 * uScale;

        ClipperOffset offset(2.0, uScale / 10000); ///*miterLimit*/ 20.0, /*roundPrecision*/ 100.0);
        offset.AddPath(curPath, jtRound, etOpenRound);
        offset.Execute(solution, size);
        if (state.imgPolarity == NEGATIVE) {
            ReversePaths(solution);
        }
    }
    return solution;
}

Paths GerberParser::CreatePolygon()
{
    Paths paths;
    double area = Area(curPath);
    if (area > 0.0) {
        if (state.imgPolarity == NEGATIVE)
            ReversePath(curPath);
    }
    else {
        if (state.imgPolarity == POSITIVE)
            ReversePath(curPath);
    }
    paths.push_back(curPath);
    return paths;
}

bool GerberParser::ParseAperture(const QString& gLine)
{
    QRegExp match(reAd);
    if (match.exactMatch(gLine) /*match.indexIn(gLine) > -1*/) {
        int apid = match.cap(1).toInt();
        QString apType = match.cap(2);
        QString apParameters = match.cap(3);
        //         Parse gerber aperture definition into dictionary of apertures.
        //         The following kinds and their attributes are supported:
        //         * *Circular (C)*: size (float)
        //         * *Rectangle (R)*: width (float), height (float)
        //         * *Obround (O)*: width (float), height (float).
        //         * *Polygon (P)*: diameter(float), vertices(int), [rotation(float)]
        //         * *Aperture Macro (AM)*: macro (ApertureMacro), modifiers (list)

        QList<QString> paramList = apParameters.split("X");

        switch (slApertureType.indexOf(apType)) {
        case CIRCULAR:
            gerbFile->apertures[apid].SetType(CIRCULAR);
            gerbFile->apertures[apid].SetDiameter(Double(paramList[0]));
            if (paramList.size() > 1) {
                gerbFile->apertures[apid].SetHoleDiameter(Double(paramList[1]));
            }
            break;
        case RECTANGLE:
            gerbFile->apertures[apid].SetType(RECTANGLE);
            gerbFile->apertures[apid].SetWidth(Double(paramList[0]));
            gerbFile->apertures[apid].SetHeight(Double(paramList[1]));
            if (paramList.size() > 2) {
                gerbFile->apertures[apid].SetHoleDiameter(Double(paramList[2]));
            }
            break;
        case OBROUND:
            gerbFile->apertures[apid].SetType(OBROUND);
            gerbFile->apertures[apid].SetWidth(Double(paramList[0]));
            gerbFile->apertures[apid].SetHeight(Double(paramList[1]));
            if (paramList.size() > 2) {
                gerbFile->apertures[apid].SetHoleDiameter(Double(paramList[2]));
            }
            break;
        case POLYGON:
            gerbFile->apertures[apid].SetType(POLYGON);
            gerbFile->apertures[apid].SetDiameter(Double(paramList[0]));
            gerbFile->apertures[apid].SetNumVertices(paramList[1].toInt());
            if (paramList.length() > 2) {
                gerbFile->apertures[apid].SetRotation(Double(paramList[2]));
            }
            if (paramList.length() > 3) {
                gerbFile->apertures[apid].SetHoleDiameter(Double(paramList[3]));
            }
            break;
        default:
            gerbFile->apertures[apid].SetType(APERTURE_MACRO);
            gerbFile->apertures[apid].SetMacro(apType);
            gerbFile->apertures[apid].SetModifiers(apertureMacro[apType].split('*'));
            for (int i = 0; i < paramList.size(); ++i) {
                gerbFile->apertures[apid].MacroCoefficients()[QString("$%1").arg(i + 1)] = Double(paramList[i], false, false);
            }
            break;
        }
        //        apertures[apid].Draw();
        return true;
    }
    return false;
}

bool GerberParser::ParseApertureMacros(const QString& gLine)
{
    QRegExp match(reAmBegin);
    if (match.exactMatch(gLine) /*match.indexIn(gLine) > -1*/) // Start macro if(match, else not an AM, carry on.
    {
        if (!match.cap(2).isEmpty() && !match.cap(3).isEmpty()) // push_back
        {
            apertureMacro[match.cap(1)] = match.cap(2);
            return true;
        }
    }
    return false;
}

bool GerberParser::ParseAttributes(const QString& gLine)
{
    QRegExp match(reAttributes);
    if (match.exactMatch(gLine) /*match.indexIn(gLine) > -1*/) {
        //        switch (slAttributeType.indexOf(match.cap(1))) {
        //        case ATTRIBUTE:
        //            //FileFunction
        //            gerberFile.attributesStrings.push_back(match.cap(2));
        //            break;
        //        case APERTURE_ATTRIBUTE:
        //            gerberFile.apertureAttributesStrings.push_back(match.cap(2));
        //            break;
        //        case OBJECT_ATTRIBUTE:
        //            gerberFile.objectAttributesStrings.push_back(match.cap(2));
        //            break;
        //        case DELETE_ATTRIBUTE:
        //            for (int i = 0; i < gerberFile.attributesStrings.size(); ++i) {
        //                if (gerberFile.attributesStrings[i].indexOf(match.cap(1)) >= 0) {
        //                    gerberFile.attributesStrings.removeAt(i);
        //                }
        //            }
        //            for (int i = 0; i < gerberFile.apertureAttributesStrings.size(); ++i) {
        //                if (gerberFile.apertureAttributesStrings[i].indexOf(match.cap(1)) >= 0) {
        //                    gerberFile.apertureAttributesStrings.removeAt(i);
        //                }
        //            }
        //            for (int i = 0; i < gerberFile.objectAttributesStrings.size(); ++i) {
        //                if (gerberFile.objectAttributesStrings[i].indexOf(match.cap(1)) >= 0) {
        //                    gerberFile.objectAttributesStrings.removeAt(i);
        //                }
        //            }
        //            break;
        //        }
        return true;
    }
    return false;
}

bool GerberParser::ParseCircularInterpolation(const QString& gLine)
{
    QRegExp match(reCirc);
    Path arcPolygon;
    double radius1, radius2, start, stop, angle;
    radius1 = radius2 = start = stop = angle = 0.0;
    if (match.exactMatch(gLine) /*match.indexIn(gLine) > -1*/) {
        if (match.cap(1).isEmpty()) {
            if (state.curGCode != G02 && state.curGCode != G03) {
                return false;
            }
        }

        cInt x, y, i, j;
        x = y = i = j = 0.0;
        if (match.cap(2).isEmpty()) {
            x = state.curPos.X;
        }
        else {
            ParseNumber(match.cap(2), x, state.format.xInteger, state.format.xDecimal);
        }

        if (match.cap(3).isEmpty()) {
            y = state.curPos.Y;
        }
        else {
            ParseNumber(match.cap(3), y, state.format.yInteger, state.format.yDecimal);
        }

        ParseNumber(match.cap(4), i, state.format.xInteger, state.format.xDecimal);
        ParseNumber(match.cap(5), j, state.format.yInteger, state.format.yDecimal);

        switch (match.cap(1).toInt()) {
        case G02:
            state.interpolation = CLOCKWISE_CIRCULAR;
            state.curGCode = G02;
            break;
        case G03:
            state.interpolation = COUNTERCLOCKWISE_CIRCULAR;
            state.curGCode = G03;
            break;
        default:
            if (state.interpolation != CLOCKWISE_CIRCULAR && state.interpolation != COUNTERCLOCKWISE_CIRCULAR) {
                qWarning() << QString("Found arc without circular interpolation mode defined. (%1)").arg(state.lineNum);
                qWarning() << QString(gLine);
                state.curPos = IntPoint(x, y);
                state.curGCode = G01;
                return false;
            }
            break;
        }

        if (state.quadrant == UNDEF) {
            qWarning() << QString("Found arc without preceding quadrant specification G74 or G75. (%1)").arg(state.lineNum);
            qWarning() << QString(gLine);
            return true;
        }

        // Set operation code if provided
        switch (match.cap(6).isEmpty() ? state.curDCode : match.cap(6).toInt()) {
        case D01:
            state.curDCode = D01;
            break;
        case D02: // Nothing created! Pen Up.
            state.curDCode = D01;
            state.curPos = IntPoint(x, y);
            ClosePath();
            return true;
        case D03: // Flash should not happen here
            state.curDCode = D02;
            state.curPos = IntPoint(x, y);
            qWarning() << QString("Trying to flash within arc. (%1)").arg(state.lineNum);
            return true;
        }

        const IntPoint centerPos[4] = {
            IntPoint(state.curPos.X + i, state.curPos.Y + j),
            IntPoint(state.curPos.X - i, state.curPos.Y + j),
            IntPoint(state.curPos.X + i, state.curPos.Y - j),
            IntPoint(state.curPos.X - i, state.curPos.Y - j)
        };
        bool valid = false;

        curPath.push_back(state.curPos);

        switch (state.quadrant) {
        case MULTI: //G75
            radius1 = sqrt(pow(i, 2.0) + pow(j, 2.0));
            start = atan2(-j, -i); // Start angle
            // Численные ошибки могут помешать, start == stop, поэтому мы проверяем заблаговременно.
            // Это должно привести к образованию дуги в 360 градусов.
            if (state.curPos == IntPoint(x, y)) {
                stop = start;
            }
            else {
                stop = atan2(-centerPos[0].Y + y, -centerPos[0].X + x); // Stop angle
            }
            arcPolygon = Arc(centerPos[0], radius1, start, stop);
            //arcPolygon = Arc2(currentPos, IntPoint(x, y), center);
            // Последняя точка в вычисленной дуге может иметь числовые ошибки.
            // Точной конечной точкой является указанная (x, y). Заменить.
            state.curPos = IntPoint(x, y);
            if (arcPolygon.size())
                arcPolygon[arcPolygon.size() - 1] = state.curPos;
            else
                arcPolygon.push_back(state.curPos);
            break;
        case SINGLE: //G74
            for (int c = 0; c < 4; ++c) {
                radius1 = sqrt(i * i + j * j);
                radius2 = sqrt(pow(centerPos[c].X - x, 2.0) + pow(centerPos[c].Y - y, 2.0));
                // Убеждаемся, что радиус начала совпадает с радиусом конца.
                if (qAbs(radius2 - radius1) > (1e-4 * uScale)) {
                    // Недействительный центр.
                    continue;
                }
                // Correct i and j and return true; as with multi-quadrant.
                i = centerPos[c].X - state.curPos.X;
                j = centerPos[c].Y - state.curPos.Y;
                // Углы
                start = atan2(-j, -i);
                stop = atan2(-centerPos[c].Y + y, -centerPos[c].X + x);
                angle = ArcAngle(start, stop);
                if (angle < (M_PI + 1e-5) * 0.5) {
                    arcPolygon = Arc(centerPos[c], radius1, start, stop);
                    // Replace with exact values
                    state.curPos = IntPoint(x, y);
                    if (arcPolygon.size())
                        arcPolygon.last() = state.curPos;
                    else
                        arcPolygon.push_back(state.curPos);
                    valid = true;
                }
            }
            if (!valid) {
                qWarning() << QString("Invalid arc in line %1.").arg(state.lineNum) << gLine;
            }
            break;
        default:
            state.curPos = IntPoint(x, y);

            curPath.push_back(state.curPos);
            return true;
            //            break;
        }
        curPath.append(arcPolygon);
        //        if (arcPolygon.size() > 0) {
        //            for (Path::size_type i = 0, size = arcPolygon.size(); i < size && size; ++i) {
        //                curPath.push_back(arcPolygon[i]); //polygon.emplace_back(arcPolygon); //push_back
        //            }
        //        }
        state.lstAperture = state.curAperture;

        return true;
    }
    return false;
}

bool GerberParser::ParseEndOfFile(const QString& gLine)
{
    QRegExp match(reEof1);
    if (match.exactMatch(gLine) /*match.indexIn(gLine) > -1*/) {
        ClosePath();
        return true;
    }
    match = QRegExp(reEof2);
    if (match.exactMatch(gLine) /*match.indexIn(gLine) > -1*/) {
        ClosePath();
        return true;
    }
    return false;
}

bool GerberParser::ParseFormat(const QString& gLine)
{
    const QStringList zeroOmissionModeList = QString("L|T").split("|");
    const QStringList coordinateValuesNotationList = QString("A|I").split("|");
    QRegExp match(reFormat);
    if (match.exactMatch(gLine) /*match.indexIn(gLine) > -1*/) {
        switch (zeroOmissionModeList.indexOf(match.cap(1))) {
        case OMIT_LEADING_ZEROS:
            state.format.zeroOmisMode = OMIT_LEADING_ZEROS;
            break;
        case OMIT_TRAILING_ZEROS:
            state.format.zeroOmisMode = OMIT_TRAILING_ZEROS;
            break;
        }
        switch (coordinateValuesNotationList.indexOf(match.cap(2))) {
        case ABSOLUTE_NOTATION:
            state.format.coordValueNotation = ABSOLUTE_NOTATION;
            break;
        case INCREMENTAL_NOTATION:
            state.format.coordValueNotation = INCREMENTAL_NOTATION;
            break;
        }
        state.format.xInteger = match.cap(3).toInt();
        state.format.xDecimal = match.cap(4).toInt();
        state.format.yInteger = match.cap(5).toInt();
        state.format.yDecimal = match.cap(6).toInt();

        int intVal = state.format.xInteger;
        if (intVal < 0 || intVal > 8) {
            throw "Modifiers '" + gLine + "' XY is out of bounds 0≤N≤7";
        }
        intVal = state.format.xDecimal;
        if (intVal < 0 || intVal > 8) {
            throw "Modifiers '" + gLine + "' XY is out of bounds 0≤N≤7";
        }
        intVal = state.format.yInteger;
        if (intVal < 0 || intVal > 8) {
            throw "Modifiers '" + gLine + "' XY is out of bounds 0≤N≤7";
        }
        intVal = state.format.yDecimal;
        if (intVal < 0 || intVal > 8) {
            throw "Modifiers '" + gLine + "' XY is out of bounds 0≤N≤7";
        }
        return true;
    }
    return false;
}

bool GerberParser::ParseGCode(const QString& gLine)
{

    QRegExp match(reGCode);
    if (match.exactMatch(gLine) /*match.indexIn(gLine) > -1*/) {
        switch (match.cap(1).toInt()) {
        case G01:
            state.interpolation = LINEAR;
            state.curGCode = G01;
            break;
        case G02:
            state.interpolation = CLOCKWISE_CIRCULAR;
            state.curGCode = G02;
            break;
        case G03:
            state.interpolation = COUNTERCLOCKWISE_CIRCULAR;
            state.curGCode = G03;
            break;
        case G04:
            state.curGCode = G04;
            break;
        case G36:
            ClosePath();
            state.region = ON;
            state.curGCode = G36;
            break;
        case G37:
            ClosePath();
            state.region = OFF;
            state.curGCode = G37;
            break;
        case G70:
            state.format.unitMode = INCHES;
            state.curGCode = G70;
            break;
        case G71:
            state.format.unitMode = MILLIMETERS;
            state.curGCode = G71;
            break;
        case G74:
            state.quadrant = SINGLE;
            state.curGCode = G74;
            break;
        case G75:
            state.quadrant = MULTI;
            state.curGCode = G75;
            break;
        case G90:
            state.format.coordValueNotation = ABSOLUTE_NOTATION;
            state.curGCode = G90;
            break;
        case G91:
            state.format.coordValueNotation = INCREMENTAL_NOTATION;
            state.curGCode = G91;
            break;
        default:
            qDebug() << "Error G-code" << match.capturedTexts();
            break;
        }
        return true;
    }
    if (QRegExp(reComment).indexIn(gLine) > -1) {
        state.curGCode = G04;
        return true;
    }
    return false;
}

bool GerberParser::ParseImagePolarity(const QString& gLine)
{
    QRegExp match(rePol);
    if (match.exactMatch(gLine) /*match.indexIn(gLine) > -1*/) {
        switch (slImagePolarity.indexOf(match.cap(1))) {
        case 0:
            state.imgPolarity = POSITIVE;
            break;
        case 1:
            state.imgPolarity = NEGATIVE;
            break;
        }
        return true;
    }
    return false;
}

bool GerberParser::ParseLevelPolarity(const QString& gLine)
{
    QRegExp match(reLpol);
    if (match.exactMatch(gLine) /*match.indexIn(gLine) > -1*/) {
        ClosePath();
        switch (slLevelPolarity.indexOf(match.cap(1))) {
        case 0:
            state.imgPolarity = POSITIVE;
            break;
        case 1:
            state.imgPolarity = NEGATIVE;
            break;
        }
        return true;
    }
    return false;
}

bool GerberParser::ParseLineInterpolation(const QString& gLine)
{
    QRegExp match(reLin);
    if (match.exactMatch(gLine) /*match.indexIn(gLine) > -1*/) {
        ParsePosition(gLine);
        switch (match.cap(2).isEmpty() ? state.curDCode : match.cap(2).toInt()) {
        case D01: //перемещение в указанную точку x-y с открытым затвором засветки
            state.curDCode = D01;

            curPath.push_back(state.curPos);
            state.lstAperture = state.curAperture;
            break;
        case D02: //перемещение в указанную точку x-y с закрытым затвором засветки
            state.curDCode = D02;
            //             if(curPath.size()&&curPath[curPath.size()-1]!=state.curPos)     curPath.push_back(state.curPos);
            ClosePath();
            break;
        case D03: //перемещение в указанную точку x-y с закрытым затвором засветки
            state.curDCode = D03;
            ClosePath();
            CreateFlash();
            break;
        default:
            break;
        }
        return true;
    }
    return false;
}

bool GerberParser::ParseOperationDCode(const QString& gLine)
{
    QRegExp match(reDCode);
    if (match.exactMatch(gLine) /*match.indexIn(gLine) > -1*/) {
        switch (match.cap(1).toInt()) {
        case D01:
            state.curDCode = D01;
            break;
        case D02:
            state.curDCode = D02;
            break;
        case D03:
            state.curDCode = D03;
            CreateFlash();
            break;
        }
        return true;
    }
    return false;
}

bool GerberParser::ParseStepAndRepeat(const QString& gLine)
{
    QRegExp match2(reStepAndRepeat);
    if (match2.indexIn(gLine) > -1) {
        return true;
    }
    return false;
}

bool GerberParser::ParseToolAperture(const QString& gLine)
{
    QRegExp match(reTool);
    if (match.exactMatch(gLine) /*match.indexIn(gLine) > -1*/) {
        state.lstAperture = state.curAperture;
        state.curAperture = match.cap(1).toInt();
        state.curDCode = D02;
        state.curGCode = G54;
        ClosePath();
        return true;
    }
    return false;
}

bool GerberParser::ParseUnitMode(const QString& gLine)
{
    QRegExp match(reUnitMode);
    if (match.exactMatch(gLine) /*match.indexIn(gLine) > -1*/) {
        switch (slUnitType.indexOf(match.cap(1))) {
        case INCHES:
            state.format.unitMode = INCHES;
            break;
        case MILLIMETERS:
            state.format.unitMode = MILLIMETERS;
            break;
        }
        return true;
    }
    return false;
}
