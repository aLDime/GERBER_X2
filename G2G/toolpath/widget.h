#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <tooldatabase/tool.h>

class QCheckBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;

enum {
    CLIMB,
    CONVENTIONAL
};

class Widget : public QWidget {
    Q_OBJECT

public:
    explicit Widget(QWidget* parent = nullptr);

public:
    virtual void calculate() = 0;
    virtual Tool getTool(int n) const = 0;
    virtual void setTool(int n, const Tool& value) = 0;

    void restoreTools(const QVector<Tool*>& tool, const QString& pathName) const;
    void saveTools(const QVector<Tool*>& tool, const QString& pathName);
    void setName(const QString& pathName);
signals:

public slots:

protected:
    QDoubleSpinBox* depth;
    QLineEdit* pathName;
    QLabel* toolName[2];
    QLabel* panelName;
    QCheckBox* cbxTool2;
};

#endif // WIDGET_H
