#include "parser.h"
#include <QDateTime>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QTextStream>
#include <QThread>

//#include <toolpath/toolpathcreator.h>

using namespace G;

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

int id1 = qRegisterMetaType<G::File*>("G::GFile*");

Parser::Parser(QObject* parent)
    : QObject(parent)
{
}

void Parser::parseFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return;

    QTextStream in(&file);
    parseLines(in.readAll(), fileName);
}

void Parser::parseLines(const QString& gerberLines, const QString& fileName)
{
    static QMutex mutex;
    mutex.lock();

    static const QRegExp match(QStringLiteral("FS([LT]?)([AI]?)X(\\d)(\\d)Y(\\d)(\\d)\\*"));
    if (match.indexIn(gerberLines) == -1) {
        emit fileError("", QFileInfo(fileName).fileName() + "\n" + "Incorrect File!\nNot contains format.");
        mutex.unlock();
        return;
    }

    reset(fileName);

    file->lines() = format(gerberLines);
    if (file->lines().isEmpty())
        emit fileError("", file->shortFileName() + "\n" + "Incorrect File!");

    emit fileProgress(file->shortFileName(), file->lines().size(), 0);
    QElapsedTimer t;
    t.start();

    lineNum = 0;
    for (const QString& gerberLine : file->lines()) {
        gerbLines.push_back(gerberLine);
        ++lineNum;
        if (!(lineNum % 1000))
            emit fileProgress(file->shortFileName(), 0, lineNum);
        try {
            //qWarning() << gerberLine;
            //            if (ParseApertureBlock(gerberLine))
            //                continue;

            if (parseApertureBlock(gerberLine))
                continue;

            if (parseEndOfFile(gerberLine))
                continue;

            if (parseGCode(gerberLine))
                continue;

            if (parseAttributes(gerberLine))
                continue;

            if (parseStepRepeat(gerberLine))
                continue; //////////

            if (parseImagePolarity(gerberLine))
                continue;

            if (parseApertureMacros(gerberLine))
                continue;

            if (parseDCode(gerberLine))
                continue;

            // Aperture definitions %ADD...
            if (parseAperture(gerberLine))
                continue;

            // Polarity change
            // Example: %LPD*% or %LPC*%
            // If polarity changes, creates geometry from current
            // buffer, then adds or subtracts accordingly.
            //            if (parseLevelPolarity(gerberLine)) //-> parseTransformations
            //                continue;
            if (parseTransformations(gerberLine))
                continue;

            // Number format
            // Example: %FSLAX24Y24*%
            // TODO: This is ignoring most of the format-> Implement the rest.
            if (parseFormat(gerberLine))
                continue;

            // Mode (IN/MM)
            // Example: %MOIN*%
            if (parseUnitMode(gerberLine))
                continue;

            // G01 - Linear interpolation plus flashes
            // Operation code (D0x) missing is deprecated... oh well I will support it.
            // REGEX: r"^(?:G0?(1))?(?:X(-?\d+))?(?:Y(-?\d+))?(?:D0([123]))?\*$"
            if (parseLineInterpolation(gerberLine))
                continue;

            // G02/3 - Circular interpolation
            // 2-clockwise, 3-counterclockwise
            if (parseCircularInterpolation(gerberLine))
                continue;

            // Line did not match any pattern. Warn user.
            // qWarning() << QString("Line ignored (%1): '%2'").arg(state.lineNum).arg(gerberLine);
        } catch (const QString& errStr) {
            qWarning() << "exeption Q:" << errStr;
            emit fileError("", file->shortFileName() + "\n" + errStr);
            break;
        } catch (const char* errStr) {
            qWarning() << "exeption Q:" << errStr;
            emit fileError("", file->shortFileName() + "\n" + errStr);
            break;
        } catch (...) {
            qWarning() << "exeption S:" << errno;
            emit fileError("", file->shortFileName() + "\n" + "Unknown Error!");
            break;
        }
    }

    if (file->isEmpty()) {
        delete file;
    } else {
        file->setItemGroup(new ItemGroup);
        for (const Paths& paths : file->groupedPaths()) {
            file->itemGroup()->append(new GerberItem(paths, file));
        }
        emit fileReady(file);
    }
    emit fileProgress(file->shortFileName(), 1, 1);

    gerbLines.clear();
    apertureMacro.clear();
    path.clear();
    mutex.unlock();
}

QList<QString> Parser::format(QString data)
{
    QList<QString> gerberLines;

    enum State {
        Param,
        Macro,
        Data,
    };

    State state = Data;
    QString lastLine;

    auto gerberLinesAppend = [&gerberLines, &lastLine](State& state, const QString& val) -> void {
        switch (state) {
        case Macro:
            lastLine.push_back(val);
            if (lastLine.endsWith('%')) {
                gerberLines << lastLine;
                state = Data;
            }
            break;
        case Param:
            lastLine.push_back(val);
            if (lastLine.endsWith('%')) {
                for (QString& tmpline : lastLine.remove('%').split('*')) {
                    if (!tmpline.isEmpty()) {
                        gerberLines << ('%' + tmpline + "*%");
                    }
                }
                state = Data;
            }
            break;
        case Data:
            break;
        }
    };

    auto lastLineClose = [&gerberLines](State state, QString& val) -> void {
        switch (state) {
        case Macro:
            if (!val.endsWith('%'))
                val.push_back('%');
            if (!val.endsWith("*%"))
                val.insert(val.length() - 2, '*');
            gerberLines << val;
            break;
        case Param:
            for (QString& tmpline : val.remove('%').split('*')) {
                if (!tmpline.isEmpty()) {
                    gerberLines << ('%' + tmpline + "*%");
                }
            }
            break;
        case Data:
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
        } else {
            gerberLines << val;
        }
    };
    for (QString& line : data.replace('\r', '\n').replace("\n\n", "\n").replace('\t', ' ').split('\n')) {
        line = line.trimmed();

        if (line.isEmpty())
            continue;

        if (line.startsWith('%') && line.endsWith('%') && line.size() > 1) {
            lastLineClose(state, lastLine);
            if (line.startsWith("%AM")) {
                lastLineClose(Macro, line);
            } else {
                lastLineClose(Param, line);
            }
            state = Data;
            continue;
        } else if (line.startsWith("%AM")) {
            lastLineClose(state, lastLine);
            state = Macro;
            lastLine = line;
            continue;
        } else if (line.startsWith('%')) {
            lastLineClose(state, lastLine);
            state = Param;
            lastLine = line;
            continue;
        } else if (line.endsWith('*') && line.length() > 1) {
            switch (state) {
            case Macro:
            case Param:
                gerberLinesAppend(state, line);
                continue;
            case Data:
                dataClose(line);
                continue;
            }
        } else {
            switch (state) {
            case Macro:
            case Param:
                gerberLinesAppend(state, line);
                continue;
            case Data:
                qWarning() << "Хрен его знает:" << line;
                continue;
            }
        }
    }
    return gerberLines;
}
#include <math.h>

#include <gi/graphicsitem.h>

double Parser::arcAngle(double start, double stop)
{
    if (state.interpolation() == CounterclockwiseCircular && stop <= start) {
        stop += 2.0 * M_PI;
    }
    if (state.interpolation() == ClockwiseCircular && stop >= start) {
        stop -= 2.0 * M_PI;
    }
    return qAbs(stop - start);
}

double Parser::toDouble(const QString& Str, bool scale, bool inchControl)
{
    bool ok;
    double d = Str.toDouble(&ok);
    if (state.format()->unitMode == Inches && inchControl)
        d *= 25.4;
    if (scale)
        d *= uScale;
    return d;
}

bool Parser::parseNumber(QString Str, cInt& val, int integer, int decimal)
{
    bool flag = false;
    int sign = 1;
    if (!Str.isEmpty()) {
        if (!decimal)
            decimal = state.format()->xDecimal;

        if (!integer)
            integer = state.format()->xInteger;

        if (Str.indexOf("+") == 0) {
            Str.remove(0, 1);
            sign = 1;
        }

        if (Str.indexOf("-") == 0) {
            Str.remove(0, 1);
            sign = -1;
        }

        if (Str.count('.'))
            Str.setNum(Str.split('.').first().toInt() + ("0." + Str.split('.').last()).toDouble());

        while (Str.length() < integer + decimal) {
            switch (state.format()->zeroOmisMode) {
            case OmitLeadingZeros:
                Str = QString(integer + decimal - Str.length(), '0') + Str;
                //Str = "0" + Str;
                break;
#ifdef DEPRECATED
            case OmitTrailingZeros:
                Str += QString(integer + decimal - Str.length(), '0');
                //Str += "0";
                break;
#endif
            }
        }
        val = toDouble(Str, true) * pow(10.0, -decimal) * sign;
        return true;
    }
    return flag;
}

void Parser::addPath()
{
    if (path.size() < 2) {
        path.clear();
        path.push_back(state.curPos());
        return;
    }

    switch (state.region()) {
    case On:
        state.setType(Region);
        switch (abSrId.top().first) {
        case Normal:
            file->append(GraphicObject(state, createPolygon(), file, path));
            break;
        case StepRepeat:
            sr.acc.append(GraphicObject(state, createPolygon(), file, path));
            break;
        case ApertureBlock:
            apb(abSrId.top().second)->append(GraphicObject(state, createPolygon(), file, path));
            break;
        }
        break;
    case Off:
        state.setType(Line);
        switch (abSrId.top().first) {
        case Normal:
            file->append(GraphicObject(state, createLine(), file, path));
            break;
        case StepRepeat:
            sr.acc.append(GraphicObject(state, createLine(), file, path));
            break;
        case ApertureBlock:
            apb(abSrId.top().second)->append(GraphicObject(state, createLine(), file, path));
            break;
        }
        break;
    }
    resetStep();
}

void Parser::addFlash()
{

    state.setType(Aperture);

    if (file->m_apertures.isEmpty() && file->m_apertures[state.aperture()] == nullptr)
        throw QString("Aperture %1 not found!").arg(state.aperture());

    Paths paths(file->m_apertures[state.aperture()]->draw(state));
    if (file->m_apertures[state.aperture()]->isDrilled())
        paths.push_back(file->m_apertures[state.aperture()]->drawDrill(state));

    switch (abSrId.top().first) {
    case Normal:
        file->append(GraphicObject(state, paths, file));
    case StepRepeat:
        sr.acc.append(GraphicObject(state, paths, file));
        break;
    case ApertureBlock:
        apb(abSrId.top().second)->append(GraphicObject(state, paths, file));
        break;
    }
    resetStep();
}

void Parser::reset(const QString& fileName)
{
    gerbLines.clear();
    apertureMacro.clear();
    path.clear();
    file = new File(fileName);
    state.reset(&file->format);

    abSrId.clear();
    abSrId.push({ Normal, 0 });
    sr.reset();
}

void Parser::resetStep()
{
    gerbLines.clear();
    path.clear();
    path.push_back(state.curPos());
}

IntPoint Parser::parsePosition(const QString& xyStr)
{
    static const QRegExp match(QStringLiteral("(?:G[01]{1,2})?(?:X([+-]?\\d*\\.?\\d+))?(?:Y([+-]?\\d*\\.?\\d+))?"));
    if (match.indexIn(xyStr) > -1) {
        cInt tmp = 0;
        if (parseNumber(match.cap(1), tmp, state.format()->xInteger, state.format()->xDecimal)) {
            state.format()->coordValueNotation == AbsoluteNotation ? state.curPos().X = tmp : state.curPos().X += tmp;
        }
        tmp = 0;
        if (parseNumber(match.cap(2), tmp, state.format()->yInteger, state.format()->yDecimal)) {
            state.format()->coordValueNotation == AbsoluteNotation ? state.curPos().Y = tmp : state.curPos().Y += tmp;
        }
    }
    if (2.0e-310 > state.curPos().X && state.curPos().X > 0.0) {
        throw QString("line num %1: '%2', error value.").arg(lineNum).arg(gerbLines.last());
    }
    if (2.0e-310 > state.curPos().Y && state.curPos().Y > 0.0) {
        throw QString("line num %1: '%2', error value.").arg(lineNum).arg(gerbLines.last());
    }
    return state.curPos();
}

Path Parser::arc(const IntPoint& center, double radius, double start, double stop)
{
    const double da_sign[4] = { 0, 0, -1.0, +1.0 };
    Path points;

    const double length = 0.5; // mm
    const int destSteps = M_PI / asin((length * 0.5) / (radius * dScale));
    int intSteps = MinStepsPerCircle;
    while (intSteps < destSteps)
        intSteps <<= 1; // aka *= 2 // Aiming for 0.5 mm rib length

    if (state.interpolation() == ClockwiseCircular && stop >= start)
        stop -= 2.0 * M_PI;
    else if (state.interpolation() == CounterclockwiseCircular && stop <= start)
        stop += 2.0 * M_PI;

    double angle = qAbs(stop - start);
    double steps = qMax((int)ceil(angle / (2.0 * M_PI) * intSteps), 2);
    double delta_angle = da_sign[state.interpolation()] * angle * 1.0 / steps;
    for (int i = 0; i < steps; i++) {
        double theta = start + delta_angle * (i + 1);
        points.push_back(IntPoint(center.X + radius * cos(theta), center.Y + radius * sin(theta)));
    }

    return points;
}

Path Parser::arc(IntPoint p1, IntPoint p2, IntPoint center)
{
    double radius = sqrt(pow((center.X - p1.X), 2) + pow((center.Y - p1.Y), 2));
    double start = atan2(p1.Y - center.Y, p1.X - center.X);
    double stop = atan2(p2.Y - center.Y, p2.X - center.X);
    return arc(center, radius, start, stop);
}

Paths Parser::createLine()
{
    Paths solution;
    if (!file->m_apertures.contains(state.aperture()))
        throw QString("Aperture %1 not found!").arg(state.aperture());

    if (file->m_apertures[state.aperture()]->type() == Rectangle) {
        State tmpState(state);
        tmpState.curPos() = IntPoint();
        Path pattern = file->m_apertures[state.aperture()]->draw(tmpState)[0];
        ReversePath(pattern);
        for (int i = 0, end = path.size() - 1; i < end; ++i) {
            int vi = 0;
            int wi = 0;
            const int nv = pattern.size();
            const int nw = 2;
            Path A;
            Path W({ path[i + 1], path[i] });
            double wAngle = Angle(W[(wi + 1) % nw], W[(wi) % nw]);
            double vAngle = Angle(pattern[(vi + 1) % nv], pattern[(vi) % nv]);
            while (vAngle > wAngle) {
                ++vi;
                vAngle = Angle(pattern[(vi + 1) % nv], pattern[(vi) % nv]);
            }
            qDebug() << vi;
            while (A.size() < (nv + nw)) {
                A.append(IntPoint(pattern[vi % nv].X + W[wi % nw].X, pattern[vi % nv].Y + W[wi % nw].Y));
                vAngle = Angle(pattern[(vi + 1) % nv], pattern[(vi) % nv]);
                wAngle = Angle(W[(wi + 1) % nw], W[(wi) % nw]);
                if (vAngle < wAngle)
                    ++vi;
                else if (vAngle > wAngle)
                    ++wi;
                else {
                    ++vi;
                    ++wi;
                }
            }
            solution.append(A);
        }
        SimplifyPolygons(solution);
        ReversePaths(solution);

        if (state.imgPolarity() == Negative)
            ReversePaths(solution);
    } else { //if (file->m_apertures[state.aperture]->type() == Circle) {        //потровится ести нет апертуры!!!!!!!
        double size = file->m_apertures[state.aperture()]->apertureSize() * uScale * 0.5 * state.scaling();
        if (qFuzzyIsNull(size))
            size = 1;
        ClipperOffset offset(2.0, uScale / 10000); ///*miterLimit*/ 20.0, /*roundPrecision*/ 100.0);
        offset.AddPath(path, jtRound, etOpenRound);
        offset.Execute(solution, size);
        if (state.imgPolarity() == Negative)
            ReversePaths(solution);
    }

    return solution;
}

Paths Parser::createPolygon()
{
    Paths paths;
    double area = Area(path);
    if (area > 0.0) {
        if (state.imgPolarity() == Negative)
            ReversePath(path);
    } else {
        if (state.imgPolarity() == Positive)
            ReversePath(path);
    }
    paths.push_back(path);
    return paths;
}

bool Parser::parseAperture(const QString& gLine)
{
    static const QRegExp match(QStringLiteral("^%ADD(\\d\\d+)([a-zA-Z_$\\.][a-zA-Z0-9_$\\.\\-]*)(?:,(.*))?\\*%$"));
    static const QList<QString> slApertureType({ "C", "R", "O", "P", "M" });
    if (match.exactMatch(gLine)) {
        int aperture = /*state.aperture =*/match.cap(1).toInt();
        QString apType = match.cap(2);
        QString apParameters = match.cap(3);

        // Parse gerber aperture definition into dictionary of apertures.
        // The following kinds and their attributes are supported:
        // * Circular (C)*: size (float)
        // * Rectangle (R)*: width (float), height (float)
        // * Obround (O)*: width (float), height (float).
        // * Polygon (P)*: diameter(float), vertices(int), [rotation(float)]
        // * Aperture Macro (AM)*: macro (ApertureMacro), modifiers (list)

        QList<QString> paramList = apParameters.split("X");
        double hole = 0.0, rotation = 0.0;
        switch (slApertureType.indexOf(apType)) {
        case Circle:
            if (paramList.size() > 1)
                hole = toDouble(paramList[1]);
            file->m_apertures[aperture] = QSharedPointer<AbstractAperture>(new ApCircle(toDouble(paramList[0]), hole, &file->format));

            break;
        case Rectangle:
            if (paramList.size() > 2)
                hole = toDouble(paramList[2]);
            file->m_apertures.insert(aperture, QSharedPointer<AbstractAperture>(new ApRectangle(toDouble(paramList[0]), toDouble(paramList[1]), hole, &file->format)));

            break;
        case Obround:
            if (paramList.size() > 2)
                hole = toDouble(paramList[2]);
            file->m_apertures.insert(aperture, QSharedPointer<AbstractAperture>(new ApObround(toDouble(paramList[0]), toDouble(paramList[1]), hole, &file->format)));
            break;
        case Polygon:
            if (paramList.length() > 2)
                rotation = toDouble(paramList[2], false, false);
            if (paramList.length() > 3)
                hole = toDouble(paramList[3]);
            file->m_apertures.insert(aperture, QSharedPointer<AbstractAperture>(new ApPolygon(toDouble(paramList[0]), paramList[1].toInt(), rotation, hole, &file->format)));

            break;
        case Macro:
        default:
            QMap<QString, double> macroCoeff;
            for (int i = 0; i < paramList.size(); ++i) {
                macroCoeff[QString("$%1").arg(i + 1)] = toDouble(paramList[i], false, false);
            }
            file->m_apertures.insert(aperture, QSharedPointer<AbstractAperture>(new ApMacro(apType, apertureMacro[apType].split('*'), macroCoeff, &file->format)));
            break;
        }
        return true;
    }
    return false;
}

bool Parser::parseApertureBlock(const QString& gLine)
{
    static const QRegExp match(QStringLiteral("^%ABD(\\d+)\\*%$"));
    if (match.exactMatch(gLine)) {
        abSrId.push({ ApertureBlock, match.cap(1).toInt() });
        file->m_apertures.insert(abSrId.top().second, QSharedPointer<AbstractAperture>(new ApBlock(&file->format)));
        return true;
    }
    if (gLine == "%AB*%") {
        addPath();
        abSrId.pop();
        return true;
    }
    return false;
}

bool Parser::parseTransformations(const QString& gLine)
{
    enum {
        trPolarity,
        trMirror,
        trRotate,
        trScale,
    };
    static const QStringList slTransformations{ "P", "M", "R", "S" };
    static const QStringList slLevelPolarity{ "D", "C" };
    static const QStringList slLoadMirroring{ "N", "X", "Y", "XY" };
    static const QRegExp match(QStringLiteral("^%L([PMRS])(.+)\\*%$"));
    if (match.exactMatch(gLine)) {
        switch (slTransformations.indexOf(match.cap(1))) {
        case trPolarity:
            addPath();
            switch (slLevelPolarity.indexOf(match.cap(2))) {
            case Positive:
                state.setImgPolarity(Positive);
                break;
            case Negative:
                state.setImgPolarity(Negative);
                break;
            default:
                throw "Polarity error!";
            }
            return true;
        case trMirror:
            state.setMirroring(static_cast<Mirroring>(slLoadMirroring.indexOf(match.cap(2))));
            return true;
        case trRotate:
            state.setRotating(match.cap(2).toDouble());
            return true;
        case trScale:
            state.setScaling(match.cap(2).toDouble());
            return true;
        }
    }
    return false;
}

bool Parser::parseStepRepeat(const QString& gLine)
{
    return false;
    //<SR open>      = %SRX<Repeats>Y<Repeats>I<Step>J<Step>*%
    //<SR close>     = %SR*%
    //<SR statement> = <SR open>{<single command>|<region statement>}<SR close>
    static const QRegExp match(QStringLiteral("^%SRX(\\d+)Y(\\d+)I([+-]?\\d*\\.?\\d+)J([+-]?\\d*\\.?\\d+)\\*%$"));
    if (match.exactMatch(gLine)) {
        if (abSrId.top().first == StepRepeat)
            closeStepRepeat();
        sr.reset();
        sr.x = match.cap(1).toInt();
        sr.y = match.cap(2).toInt();
        sr.i = match.cap(3).toDouble() * uScale;
        sr.j = match.cap(4).toDouble() * uScale;
        if (state.format()->unitMode == Inches) {
            sr.i *= 25.4;
            sr.j *= 25.4;
        }
        if (sr.x > 1 || sr.y > 1)
            abSrId.push({ StepRepeat, 0 });
        return true;
    }
    QRegExp match2("^%SR\\*%$");
    if (match2.exactMatch(gLine)) {
        if (abSrId.top().first == StepRepeat)
            closeStepRepeat();
        return true;
    }
    return false;
}

void Parser::closeStepRepeat()
{
    qDebug() << "sr" << sr.x << sr.y;
    for (int y = 0; y < sr.y; ++y) {
        for (int x = 0; x < sr.x; ++x) {
            for (GraphicObject go : sr.acc) {
                for (Path& path : go.paths) {
                    AbstractAperture::translate(path, IntPoint(sr.i * x, sr.j * y));
                }
                file->append(go);
            }
        }
    }
    sr.reset();
    abSrId.pop();
}

bool Parser::parseApertureMacros(const QString& gLine)
{
    static const QRegExp match(QStringLiteral("^%AM([^\\*]+)\\*([^%]+)?(%)?$"));
    // Start macro if(match, else not an AM, carry on.
    if (match.exactMatch(gLine)) {
        if (!match.cap(2).isEmpty() && !match.cap(3).isEmpty()) {
            apertureMacro[match.cap(1)] = match.cap(2);
            return true;
        }
    }
    return false;
}

bool Parser::parseAttributes(const QString& gLine)
{
    static const QRegExp match(QStringLiteral("^%(TF|TA|TO|TD)(.*)\\*%$"));
    if (match.exactMatch(gLine)) {
        //const QList<QString> slAttributeType(QString("TF|TA|TO|TD").split("|"));
        // switch (slAttributeType.indexOf(match.cap(1))) {
        // case ATTRIBUTE:
        // //FileFunction
        // gerberFile.attributesStrings.push_back(match.cap(2));
        // break;
        // case APERTURE_ATTRIBUTE:
        // gerberFile.apertureAttributesStrings.push_back(match.cap(2));
        // break;
        // case OBJECT_ATTRIBUTE:
        // gerberFile.objectAttributesStrings.push_back(match.cap(2));
        // break;
        // case DELETE_ATTRIBUTE:
        // for (int i = 0; i < gerberFile.attributesStrings.size(); ++i) {
        // if (gerberFile.attributesStrings[i].indexOf(match.cap(1)) >= 0) {
        // gerberFile.attributesStrings.removeAt(i);
        // }
        // }
        // for (int i = 0; i < gerberFile.apertureAttributesStrings.size(); ++i) {
        // if (gerberFile.apertureAttributesStrings[i].indexOf(match.cap(1)) >= 0) {
        // gerberFile.apertureAttributesStrings.removeAt(i);
        // }
        // }
        // for (int i = 0; i < gerberFile.objectAttributesStrings.size(); ++i) {
        // if (gerberFile.objectAttributesStrings[i].indexOf(match.cap(1)) >= 0) {
        // gerberFile.objectAttributesStrings.removeAt(i);
        // }
        // }
        // break;
        // }
        return true;
    }
    return false;
}

bool Parser::parseCircularInterpolation(const QString& gLine)
{
    static const QRegExp match(QStringLiteral("^(?:G0?([23]))?[X]?([\\+-]?\\d+)*[Y]?([\\+-]?\\d+)*[I]?([\\+-]?\\d+)*[J]?([\\+-]?\\d+)*[^D]*(?:D0?([12]))?\\*$"));
    Path arcPolygon;
    double radius1, radius2, start, stop, angle;
    radius1 = radius2 = start = stop = angle = 0.0;
    if (match.exactMatch(gLine)) {
        if (match.cap(1).isEmpty()) {
            if (state.gCode() != G02 && state.gCode() != G03) {
                return false;
            }
        }

        cInt x, y, i, j;
        x = y = i = j = 0.0;
        if (match.cap(2).isEmpty()) {
            x = state.curPos().X;
        } else {
            parseNumber(match.cap(2), x, state.format()->xInteger, state.format()->xDecimal);
        }

        if (match.cap(3).isEmpty()) {
            y = state.curPos().Y;
        } else {
            parseNumber(match.cap(3), y, state.format()->yInteger, state.format()->yDecimal);
        }

        parseNumber(match.cap(4), i, state.format()->xInteger, state.format()->xDecimal);
        parseNumber(match.cap(5), j, state.format()->yInteger, state.format()->yDecimal);

        switch (match.cap(1).toInt()) {
        case G02:
            state.setInterpolation(ClockwiseCircular);
            state.setGCode(G02);
            break;
        case G03:
            state.setInterpolation(CounterclockwiseCircular);
            state.setGCode(G03);
            break;
        default:
            if (state.interpolation() != ClockwiseCircular && state.interpolation() != CounterclockwiseCircular) {
                qWarning() << QString("Found arc without circular interpolation mode defined. (%1)").arg(lineNum);
                qWarning() << QString(gLine);
                state.setCurPos({ x, y });
                state.setGCode(G01);
                return false;
            }
            break;
        }

        if (state.quadrant() == Undef) {
            qWarning() << QString("Found arc without preceding quadrant specification G74 or G75. (%1)").arg(lineNum);
            qWarning() << QString(gLine);
            return true;
        }

        // Set operation code if provided
        if (!match.cap(6).isEmpty())
            state.setDCode(static_cast<DCode>(match.cap(6).toInt()));
        switch (state.dCode()) {
        case D01:
            break;
        case D02: // Nothing created! Pen Up.
            state.setDCode(D01);
            state.setCurPos({ x, y });
            addPath();
            return true;
        case D03: // Flash should not happen here
            state.setCurPos({ x, y });
            qWarning() << QString("Trying to flash within arc. (%1)").arg(lineNum);
            return true;
        }

        const IntPoint& curPos = state.curPos();

        const IntPoint centerPos[4] = {
            IntPoint(curPos.X + i, curPos.Y + j),
            IntPoint(curPos.X - i, curPos.Y + j),
            IntPoint(curPos.X + i, curPos.Y - j),
            IntPoint(curPos.X - i, curPos.Y - j)
        };

        bool valid = false;

        path.push_back(state.curPos());

        switch (state.quadrant()) {
        case Multi: //G75
            radius1 = sqrt(pow(i, 2.0) + pow(j, 2.0));
            start = atan2(-j, -i); // Start angle
            // Численные ошибки могут помешать, start == stop, поэтому мы проверяем заблаговременно.
            // Это должно привести к образованию дуги в 360 градусов.
            if (state.curPos() == IntPoint(x, y)) {
                stop = start;
            } else {
                stop = atan2(-centerPos[0].Y + y, -centerPos[0].X + x); // Stop angle
            }
            arcPolygon = arc(centerPos[0], radius1, start, stop);
            //arcPolygon = Arc2(currentPos, IntPoint(x, y), center);
            // Последняя точка в вычисленной дуге может иметь числовые ошибки.
            // Точной конечной точкой является указанная (x, y). Заменить.
            state.curPos() = IntPoint(x, y);
            if (arcPolygon.size())
                arcPolygon[arcPolygon.size() - 1] = state.curPos();
            else
                arcPolygon.push_back(state.curPos());
            break;
        case Single: //G74
            for (int c = 0; c < 4; ++c) {
                radius1 = sqrt(i * i + j * j);
                radius2 = sqrt(pow(centerPos[c].X - x, 2.0) + pow(centerPos[c].Y - y, 2.0));
                // Убеждаемся, что радиус начала совпадает с радиусом конца.
                if (qAbs(radius2 - radius1) > (1e-4 * uScale)) {
                    // Недействительный центр.
                    continue;
                }
                // Correct i and j and return true; as with multi-quadrant.
                i = centerPos[c].X - state.curPos().X;
                j = centerPos[c].Y - state.curPos().Y;
                // Углы
                start = atan2(-j, -i);
                stop = atan2(-centerPos[c].Y + y, -centerPos[c].X + x);
                angle = arcAngle(start, stop);
                if (angle < (M_PI + 1e-5) * 0.5) {
                    arcPolygon = arc(centerPos[c], radius1, start, stop);
                    // Replace with exact values
                    state.setCurPos({ x, y });
                    if (arcPolygon.size())
                        arcPolygon.last() = state.curPos();
                    else
                        arcPolygon.push_back(state.curPos());
                    valid = true;
                }
            }
            if (!valid) {
                qWarning() << QString("Invalid arc in line %1.").arg(lineNum) << gLine;
            }
            break;
        default:
            state.setCurPos({ x, y });
            path.push_back(state.curPos());
            return true;
            // break;
        }
        path.append(arcPolygon);
        // if (arcPolygon.size() > 0) {
        // for (Path::size_type i = 0, size = arcPolygon.size(); i < size && size; ++i) {
        // curPath.push_back(arcPolygon[i]); //polygon.emplace_back(arcPolygon); //push_back
        // }
        // }
        return true;
    }
    return false;
}

bool Parser::parseEndOfFile(const QString& gLine)
{
    static const QRegExp match(QStringLiteral("^M[0]?[0123]\\*"));
    if (match.exactMatch(gLine)) {
        addPath();
        return true;
    }
    static const QRegExp match2(QStringLiteral("^D0?2M0?[02]\\*"));
    if (match2.exactMatch(gLine)) {
        addPath();
        return true;
    }
    return false;
}

bool Parser::parseFormat(const QString& gLine)
{
    const QStringList zeroOmissionModeList = QString("L|T").split("|");
    const QStringList coordinateValuesNotationList = QString("A|I").split("|");
    static const QRegExp match(QStringLiteral("^%FS([LT]?)([AI]?)X(\\d)(\\d)Y(\\d)(\\d)\\*%$"));
    if (match.exactMatch(gLine)) {
        switch (zeroOmissionModeList.indexOf(match.cap(1))) {
        case OmitLeadingZeros:
            state.format()->zeroOmisMode = OmitLeadingZeros;
            break;
#ifdef DEPRECATED
        case OmitTrailingZeros:
            state.format()->zeroOmisMode = OmitTrailingZeros;
            break;
#endif
        }
        switch (coordinateValuesNotationList.indexOf(match.cap(2))) {
        case AbsoluteNotation:
            state.format()->coordValueNotation = AbsoluteNotation;
            break;
#ifdef DEPRECATED
        case IncrementalNotation:
            state.format()->coordValueNotation = IncrementalNotation;
            break;
#endif
        }
        state.format()->xInteger = match.cap(3).toInt();
        state.format()->xDecimal = match.cap(4).toInt();
        state.format()->yInteger = match.cap(5).toInt();
        state.format()->yDecimal = match.cap(6).toInt();

        int intVal = state.format()->xInteger;
        if (intVal < 0 || intVal > 8) {
            throw "Modifiers '" + gLine + "' XY is out of bounds 0≤N≤7";
        }
        intVal = state.format()->xDecimal;
        if (intVal < 0 || intVal > 8) {
            throw "Modifiers '" + gLine + "' XY is out of bounds 0≤N≤7";
        }
        intVal = state.format()->yInteger;
        if (intVal < 0 || intVal > 8) {
            throw "Modifiers '" + gLine + "' XY is out of bounds 0≤N≤7";
        }
        intVal = state.format()->yDecimal;
        if (intVal < 0 || intVal > 8) {
            throw "Modifiers '" + gLine + "' XY is out of bounds 0≤N≤7";
        }
        return true;
    }
    return false;
}

bool Parser::parseGCode(const QString& gLine)
{

    static const QRegExp match(QStringLiteral("^G([0]?[0-9]{2})\\*$"));
    if (match.exactMatch(gLine)) {
        switch (match.cap(1).toInt()) {
        case G01:
            state.setInterpolation(Linear);
            state.setGCode(G01);
            break;
        case G02:
            state.setInterpolation(ClockwiseCircular);
            state.setGCode(G02);
            break;
        case G03:
            state.setInterpolation(CounterclockwiseCircular);
            state.setGCode(G03);
            break;
        case G04:
            state.setGCode(G04);
            break;
        case G36:
            addPath();
            state.setRegion(On);
            state.setGCode(G36);
            state.setDCode(D02);
            break;
        case G37:
            addPath();
            state.setRegion(Off);
            state.setGCode(G37);
            break;
#ifdef DEPRECATED
        case G70:
            state.format()->unitMode = Inches;
            state.setGCode(G70);
            break;
        case G71:
            state.format()->unitMode = Millimeters;
            state.setGCode(G71);
            break;
#endif
        case G74:
            state.setQuadrant(Single);
            state.setGCode(G74);
            break;
        case G75:
            state.setQuadrant(Multi);
            state.setGCode(G75);
            break;
#ifdef DEPRECATED
        case G90:
            state.format()->coordValueNotation = AbsoluteNotation;
            state.setGCode(G90);
            break;
        case G91:
            state.format()->coordValueNotation = IncrementalNotation;
            state.setGCode(G91);
#endif
            break;
        default:
            qWarning() << "Erroror or deprecated G-code " << match.capturedTexts();
            break;
        }
        return true;
    }
    if (QRegExp("^G0?4(.*)$").exactMatch(gLine)) {
        state.setGCode(G04);
        return true;
    }
    return false;
}

bool Parser::parseImagePolarity(const QString& gLine)
{
    static const QRegExp match(QStringLiteral("^%IP(POS|NEG)\\*%$"));
    static const QList<QString> slImagePolarity(QString("POS|NEG").split("|"));
    if (match.exactMatch(gLine)) {
        switch (slImagePolarity.indexOf(match.cap(1))) {
        case Positive:
            state.setImgPolarity(Positive);
            break;
        case Negative:
            state.setImgPolarity(Negative);
            break;
        }
        return true;
    }
    return false;
}

bool Parser::parseLineInterpolation(const QString& gLine)
{
    static const QRegExp match(QStringLiteral("^(?:G0?(1))?(?=.*X([\\+-]?\\d+))?(?=.*Y([\\+-]?\\d+))?[XY]*[^DIJ]*(?:D0?([123]))?\\*$"));
    if (match.exactMatch(gLine)) {
        parsePosition(gLine);
        if (!match.cap(2).isEmpty())
            state.setDCode(static_cast<DCode>(match.cap(2).toInt()));
        switch (/*match.cap(2).isEmpty() ? */ state.dCode() /*: match.cap(2).toInt()*/) {
        case D01: //перемещение в указанную точку x-y с открытым затвором засветки
            path.push_back(state.curPos());
            break;
        case D02: //перемещение в указанную точку x-y с закрытым затвором засветки
            addPath();
            break;
        case D03: //перемещение в указанную точку x-y с закрытым затвором засветки и вспышка
            addPath();
            addFlash();
            break;
        default:
            break;
        }
        return true;
    }
    return false;
}

bool Parser::parseDCode(const QString& gLine)
{
    static const QRegExp match(QStringLiteral("^D0?([123])\\*$"));
    if (match.exactMatch(gLine)) {
        switch (match.cap(1).toInt()) {
        case D01:
            state.setDCode(D01);
            break;
        case D02:
            state.setDCode(D02);
            break;
        case D03:
            state.setDCode(D03);
            addFlash();
            break;
        }
        return true;
    }
    static const QRegExp match2(QStringLiteral("^(?:G54)?D(\\d+)\\*$"));
    if (match2.exactMatch(gLine)) {
        addPath();
        state.setAperture(match2.cap(1).toInt());
        state.setDCode(D02);
#ifdef DEPRECATED
        state.setGCode(G54);
#endif
        addPath();
        return true;
    }
    return false;
}

bool Parser::parseUnitMode(const QString& gLine)
{
    static const QRegExp match(QStringLiteral("^%MO(IN|MM)\\*%$"));
    static const QList<QString> slUnitType(QString("IN|MM").split("|"));
    if (match.exactMatch(gLine)) {
        switch (slUnitType.indexOf(match.cap(1))) {
        case Inches:
            state.format()->unitMode = Inches;
            break;
        case Millimeters:
            state.format()->unitMode = Millimeters;
            break;
        }
        return true;
    }
    return false;
}
