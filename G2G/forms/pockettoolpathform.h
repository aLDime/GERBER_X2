#ifndef POCKETTOOLPATHFORM_H
#define POCKETTOOLPATHFORM_H

#include "toolpathforminterface.h"

#include <QWidget>

namespace Ui {
class PocketToolpathForm;
}

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class ToolpathNameForm;

class PocketToolpathForm : public QWidget, public virtual ToolpathFormInterface {
    Q_OBJECT
public:
    explicit PocketToolpathForm(QWidget* parent = nullptr);
    ~PocketToolpathForm();

protected:
    void changeEvent(QEvent* e);
//    virtual void showEvent(QShowEvent*) override;

private slots:

private:
    QCheckBox* checkBox;
    QComboBox* comboBox;
    QDoubleSpinBox* dsbDepth;
    QDoubleSpinBox* dsbRasterAngle;
    QGroupBox* groupBox_1;
    QGroupBox* groupBox_2;
    QGroupBox* groupBox_3;
    QLabel* label;
    QLabel* label_12;
    QLabel* label_13;
    QLabel* label_14;
    QLabel* lblPixmap;
    QLabel* label_16;
    QLabel* label_17;
    QLabel* label_2;
    QLabel* label_3;
    QLabel* label_4;
    QLabel* label_5;
    QLabel* label_6;
    QLineEdit* leName;
    QPushButton* pbCalculate;
    QPushButton* pbClose;
    QPushButton* pushButton_3;
    QPushButton* pushButton_4;
    QPushButton* pushButton_5;
    QPushButton* pushButton_6;
    QRadioButton* rbClimb;
    QRadioButton* rbConventional;
    QRadioButton* rbOffset;
    QRadioButton* rbRaster;

    ToolpathNameForm* nameForm;

    QList<QRadioButton*> rbList;

    int type = 0;
    int direction = 0;

    enum {
        OFFSET,
        RASTER,
        CLIMB,
        CONVENTIONAL
    };

    void setupUi(QWidget* Form); // setupUi

    void retranslateUi(QWidget* Form); // retranslateUi

    void calculate();
};

#endif // POCKETTOOLPATHFORM_H
