#ifndef PROFILETOOLPATHFORM_H
#define PROFILETOOLPATHFORM_H

#include "toolpathforminterface.h"

#include <QWidget>

namespace Ui {
class ProfileToolpathForm;
}

class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class ToolpathNameForm;

class ProfileToolpathForm : public QWidget, public virtual ToolpathFormInterface {
    Q_OBJECT

public:
    explicit ProfileToolpathForm(QWidget* parent = 0);
    ~ProfileToolpathForm();

protected:
    void changeEvent(QEvent* e);
    virtual void showEvent(QShowEvent*) override;

private slots:
    void calculate();

private:
    //    QDoubleSpinBox* doubleSpinBox_2;
    //    QDoubleSpinBox* doubleSpinBox_3;
    //    QLabel* label_7;
    //    QLabel* label_8;
    //    QPushButton* pushButton_2;
    QDoubleSpinBox* dsbDepth;
    QGroupBox* groupBox_1;
    QGroupBox* groupBox_2;
    QGroupBox* groupBox_3;
    QGroupBox* groupBox_4;
    QLabel* label;
    QLabel* label_15;
    QLabel* label_2;
    QLabel* label_3;
    QLabel* label_5;
    QLabel* label_6;
    QLabel* lblPixmap;
    QLabel* lblToolName;
    QPushButton* pbCalculate;
    QPushButton* pbClose;
    QPushButton* pbEditTool;
    QPushButton* pbSelectTool;
    QRadioButton* rbClimb;
    QRadioButton* rbConventional;
    QRadioButton* rbInside;
    QRadioButton* rbOn;
    QRadioButton* rbOutside;
    ToolpathNameForm* nameForm;

    QList<QRadioButton*> rbList;

    int side = 0;
    int direction = 0;

    enum {
        OUTSIDE,
        INSIDE,
        ON,
        CLIMB,
        CONVENTIONAL
    };

    void setupUi(QWidget* Form); // setupUi

    void retranslateUi(QWidget* Form); // retranslateUi
};

#endif // PROFILETOOLPATHFORM_H
