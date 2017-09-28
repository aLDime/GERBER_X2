#include "mathparser.h"
#include <QDebug>
#include <QtMath>

MathParser::MathParser(QMap<QString, double>& variables)
    : variables(&variables)
{
}

MathParser::MathParser()
{
    //*variables = new QMap<QString, double>();
}

void MathParser::setVariable(QString variableName, double variableValue)
{
    variables->insert(variableName, variableValue) /*= variableValue*/;
}

double MathParser::getVariable(QString variableName)
{

    if (!variables->contains(variableName)) {
        qWarning() << "Error: Try get unexists variable '" + variableName + "'";
        return 0.0;
    }
    return variables->value(variableName, 0.0);
}

double MathParser::Parse(const QString& s)
{
    Result result(0.0, "");
    try {
        result = PlusMinus(s);
        if (!result.rest.isEmpty()) {
            qWarning() << "Error: can't full parse"
                       << "\"" << s << "\"";
            qWarning() << "rest: " + result.rest;
        }
    } catch (const QString& str) {
        qWarning() << str;
    }
    return result.acc;
}

void MathParser::setVariables(QMap<QString, double>& value)
{
    variables = &value;
}

Result MathParser::PlusMinus(QString s) //throws Exception
{
    Result current = MulDiv(s);
    double acc = current.acc;

    while (current.rest.length() > 0) {
        if (!(current.rest.at(0) == '+' || current.rest.at(0) == '-')) {
            break;
        }

        QChar sign = current.rest.at(0);
        QString next = current.rest.mid(1);

        current = MulDiv(next);
        if (sign == '+') {
            acc += current.acc;
        } else {
            acc -= current.acc;
        }
    }
    return Result(acc, current.rest);
}

Result MathParser::Bracket(QString s) //throws Exception
{
    QChar zeroChar = s.at(0);
    if (zeroChar == '(') {
        Result r = PlusMinus(s.mid(1));
        if (!r.rest.isEmpty() && r.rest.at(0) == ')') {
            r.rest = r.rest.mid(1);
        } else {
            qWarning() << "Error: not close bracket";
        }
        return r;
    }
    return FunctionVariable(s);
}

Result MathParser::FunctionVariable(QString s) //throws Exception
{
    QString f = "";
    int i = 0;
    // ищем название функции или переменной
    // имя обязательно должна начинаться с буквы
    while (i < s.length() && ((s.at(i).isLetter() || s.at(i) == '$') || (s.at(i).isDigit() && i > 0))) {
        //while (i < s.length() && (s.at(i).isLetter() || (s.at(i).isDigit() && i > 0))) {
        f += s.at(i);
        i++;
    }
    if (!f.isEmpty()) { // если что-нибудь нашли
        if (s.length() > i && s.at(i) == '(') { // и следующий символ скобка значит - это функция
            Result r = Bracket(s.mid(f.length()));
            return processFunction(f, r);
        } else { // иначе - это переменная
            return Result(getVariable(f), s.mid(f.length()));
        }
    }
    return Num(s);
}

Result MathParser::MulDiv(QString s) //throws Exception
{
    Result current = Bracket(s);

    double acc = current.acc;
    while (true) {
        if (current.rest.length() == 0) {
            return current;
        }
        QChar sign = current.rest.at(0);
        if ((sign != '*' && sign != '/')) {
            return current;
        }

        QString next = current.rest.mid(1);
        Result right = Bracket(next);

        if (sign == '*') {
            acc *= right.acc;
        } else {
            acc /= right.acc;
        }

        current = Result(acc, right.rest);
    }
}

Result MathParser::Num(QString s) //throws Exception
{
    int i = 0;
    int dot_cnt = 0;
    bool negative = false;
    // число также может начинаться с минуса
    if (s.at(0) == '-') {
        negative = true;
        s = s.mid(1);
    }
    // разрешаем только цифры и точку
    while (i < s.length() && (s.at(i).isDigit() || s.at(i) == '.')) {
        // но также проверям, что в числе может быть только одна точка!
        if (s.at(i) == '.' && ++dot_cnt > 1) {
            throw "not valid number '" + s.mid(0, i + 1) + "'";
        }
        i++;
    }
    if (i == 0) { // что-либо похожее на число мы не нашли
        throw "can't get valid number in '" + s + "'";
    }

    double dPart = s.mid(0, i).toDouble();
    if (negative) {
        dPart = -dPart;
    }
    QString restPart = s.mid(i);

    return Result(dPart, restPart);
}

Result MathParser::processFunction(QString func, Result r)
{
    enum {
        sin,
        cos,
        tan
    };
    switch (QString("sin,cos,tan").split(',').indexOf(func)) {
    case sin:
        return Result(qSin(r.acc), r.rest);
    case cos:
        return Result(qCos(r.acc), r.rest);
    case tan:
        return Result(qTan(r.acc), r.rest);
    default:
        qWarning() << "function '" + func + "' is not defined";
        break;
    }
    return r;
}
