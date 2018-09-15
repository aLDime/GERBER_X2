/**
 * https://habrahabr.ru/post/122397/
 * @author shurik
 */

#ifndef MATCHPARSER_H
#define MATCHPARSER_H

#include <QObject>
#include <QMap>

class Result {
public:
    double acc; // Аккамулятор
    QString rest; // остаток строки, которую мы еще не обработали
    Result(double v = 0.0, const QString& r = "")
        : acc(v)
        , rest(r)
    {
    }
};

class MathParser {
public:
    MathParser(QMap<QString, double>& variables);
    double getVariable(QString variableName);
    double parse(const QString& s = "");

private:
    QMap<QString, double>* variables;
    Result plusMinus(QString s);
    Result bracket(QString s);
    Result functionVariable(QString s);
    Result mulDiv(QString s);
    Result num(QString s);
    // Тут определяем все нашие функции, которыми мы можем пользоватся в формулах
    Result processFunction(QString func, Result r);
};

#endif // MATCHPARSER_H
