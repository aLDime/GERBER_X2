#ifndef PROFILEWIDGET_H
#define PROFILEWIDGET_H

#include "widget.h"

class QGridLayout;
class QLabel;
class QGroupBox;
class QVBoxLayout;
class QRadioButton;

class ProfileWidget : public Widget {
    Q_OBJECT
public:
    explicit ProfileWidget(QWidget* parent = nullptr);
    ~ProfileWidget();
signals:

public slots:

private:
    static Tool tool;

    QGridLayout* gridLayout;
    QGroupBox* groupBox;
    QGroupBox* groupBox_2;
    QLabel* lblPixmap;
    QRadioButton* rbClimb;
    QRadioButton* rbConventional;
    QRadioButton* rbInside;
    QRadioButton* rbOn;
    QRadioButton* rbOutside;
    QVBoxLayout* verticalLayout;
    QVBoxLayout* verticalLayout_2;

    void setupUi(QWidget* Form); // setupUi
    void retranslateUi(QWidget* Form); // retranslateUi

    int side = 0;
    int direction = 0;

    // Widget interface
public:
    void calculate() override;
    Tool getTool(int n) const override;
    void setTool(int n, const Tool& value) override;
};

#endif // PROFILEWIDGET_H
