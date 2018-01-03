#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <tooldatabase/tool.h>

class QDoubleSpinBox;
class QLineEdit;
class QLabel;

class Widget : public QWidget {
    Q_OBJECT

public:
    explicit Widget(QWidget* parent = nullptr);

public:
    virtual void calculate() = 0;
    virtual Tool getTool(int n) const = 0;
    virtual void setTool(int n, const Tool& value) = 0;

    void restoreTool(const QVector<Tool*>& tool, const QString& pathName) const;
    void saveTool(const QVector<Tool*>& tool, const QString& pathName);
    void setName(const QString& pathName);
signals:

public slots:

protected:
    QDoubleSpinBox* depth;
    QLineEdit* pathName;
    QLabel* toolName[2];
    QLabel* panelName;
};

#endif // WIDGET_H
