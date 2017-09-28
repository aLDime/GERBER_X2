#ifndef DRILLINGTOOLPATHFORM_H
#define DRILLINGTOOLPATHFORM_H

#include "toolpathforminterface.h"

#include <QWidget>

namespace Ui {
class DrillingToolpathForm;
}

class QCheckBox;
class QDoubleSpinBox;
class QFrame;
class QGroupBox;
class QLabel;
class QPushButton;

class DrillingToolpathForm : public QWidget, public virtual ToolpathFormInterface {
    Q_OBJECT
public:
    explicit DrillingToolpathForm(QWidget* parent = nullptr);
    ~DrillingToolpathForm();

protected:
    void changeEvent(QEvent* e);
//    virtual void showEvent(QShowEvent*) override;

private:
    QCheckBox* chbPeckDrilling;
    QDoubleSpinBox* dsbDepth;
    QDoubleSpinBox* dsbGap;
    QFrame* line;
    QFrame* line_2;
    QGroupBox* groupBox_1;
    QGroupBox* groupBox_2;
    QGroupBox* groupBox_3;
    QLabel* label;
    QLabel* label_2;
    QLabel* label_3;
    QLabel* label_4;
    QLabel* label_5;
    QPushButton* pbCalculate;
    QPushButton* pbClose;
    QPushButton* pbEdit;
    QPushButton* pbSelect;
    void setupUi(QWidget* Form); // setupUi
    void retranslateUi(QWidget* Form); // retranslateUi
};

#endif // DRILLINGTOOLPATHFORM_H
