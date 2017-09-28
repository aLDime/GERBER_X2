#ifndef TOOLPATHNAMEFORM_H
#define TOOLPATHNAMEFORM_H

#include <QWidget>

class QLabel;
class QLineEdit;

class ToolpathNameForm : public QWidget {
    Q_OBJECT
public:
    explicit ToolpathNameForm(QWidget* parent = nullptr);

    QString text() const;
    void setText(const QString& value);

protected:
    void changeEvent(QEvent* e);

private:
    void retranslateUi(QWidget* Form);
    QLabel* label_10;
    QLabel* label_11;
    QLabel* label_7;
    QLabel* label_8;
    QLabel* label_9;
    QLineEdit* leName;
};

#endif // TOOLPATHNAMEFORM_H
