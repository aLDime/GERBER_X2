#ifndef POCKETWIDGET_H
#define POCKETWIDGET_H

#include "widget.h"

class QComboBox;
class QDoubleSpinBox;
class QFormLayout;
class QGridLayout;
class QGroupBox;
class QLabel;
class QRadioButton;
class QVBoxLayout;

class PocketWidget : public Widget {
    Q_OBJECT
public:
    explicit PocketWidget(QWidget* parent = nullptr);
    ~PocketWidget();

signals:

public slots:
private:
    QComboBox* cbxProfilePass;
    QDoubleSpinBox* dsbxRasterAngle;
    QFormLayout* formLayout;
    QGridLayout* gridLayout;
    QGroupBox* groupBox;
    QGroupBox* groupBox_2;
    QLabel* label_2;
    QLabel* label_3;
    QLabel* lblPixmap;
    QRadioButton* rbClimb;
    QRadioButton* rbConventional;
    QRadioButton* rbOffset;
    QRadioButton* rbRaster;
    QVBoxLayout* verticalLayout;
    QVBoxLayout* verticalLayout_2;

    void setupUi(QWidget* Form); // setupUi

    void retranslateUi(QWidget* Form); // retranslateUi

    Tool tool1;
    Tool tool2;

    int type = 0;
    int direction = 0;

    // Widget interface
public:
    void calculate() override;
    Tool getTool(int n) const override;
    void setTool(int n, const Tool& value) override;
};

#endif // POCKETWIDGET_H
