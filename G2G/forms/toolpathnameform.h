#ifndef TOOLPATHNAMEFORM_H
#define TOOLPATHNAMEFORM_H

#include <QWidget>

#include <tooldatabase/tool.h>

class QLabel;
class QLineEdit;
class QSpinBox;

class ToolpathNameForm : public QWidget {
    Q_OBJECT
public:
    explicit ToolpathNameForm(QWidget* parent = nullptr);
    ~ToolpathNameForm();

    QString text() const;
    void setText(const QString& value);

    void getTool(Tool& tool, const QString& name) const;
    void setTool(const Tool& tool, const QString& name);

protected:
    void changeEvent(QEvent* e);

private:
    void retranslateUi(QWidget* Form);
    QLabel* label_11;
    QLabel* label_7;
    QSpinBox* dsbSafeZ;
    QSpinBox* dsbHomeZ;
    QLabel* label_9;
    QLineEdit* leName;
};

#endif // TOOLPATHNAMEFORM_H
