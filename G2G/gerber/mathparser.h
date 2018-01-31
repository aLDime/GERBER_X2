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
    //    void setVariable(QString variableName, double variableValue) { variables->insert(variableName, variableValue) /*= variableValue*/; }
    //    void setVariables(QMap<QString, double>& value) { variables = &value; }
    double getVariable(QString variableName);
    double Parse(const QString& s = "");

private:
    QMap<QString, double>* variables;
    Result PlusMinus(QString s);
    Result Bracket(QString s);
    Result FunctionVariable(QString s);
    Result MulDiv(QString s);
    Result Num(QString s);
    // Тут определяем все нашие функции, которыми мы можем пользоватся в формулах
    Result processFunction(QString func, Result r);
};

#endif // MATCHPARSER_H
