#ifndef MATERIALSETUP_H
#define MATERIALSETUP_H

#include <QWidget>

class QVBoxLayout;
class QGroupBox;
class QLabel;
class QFormLayout;
class QDoubleSpinBox;
class QFrame;
class QPushButton;
class QSpacerItem;

class MaterialSetup : public QWidget {
    Q_OBJECT
public:
    explicit MaterialSetup(QWidget* parent = nullptr);
    ~MaterialSetup();

    static MaterialSetup* This();

    void setHomePos(QPointF pos);
    void setZeroPos(QPointF pos);

    static QPointF homePos;
    static QPointF zeroPos;
    static double z;
    static double thickness;
    static double clearence;
    static double plunge;

signals:

public slots:

private:
    static bool isCreated;
    static MaterialSetup* t;

    QDoubleSpinBox* dsbxThickness;
    QDoubleSpinBox* dsbxClearence;
    QDoubleSpinBox* dsbxPlunge;
    QDoubleSpinBox* dsbxHomeX;
    QDoubleSpinBox* dsbxHomeY;
    QDoubleSpinBox* dsbxHomeZ;
    QDoubleSpinBox* dsbxZeroX;
    QDoubleSpinBox* dsbxZeroY;
    QDoubleSpinBox* dsbxZeroZ;

    QFormLayout* formLayout;
    QFrame* line;
    QFrame* line_2;
    QFrame* line_3;
    QGroupBox* groupBox;
    QGroupBox* groupBox_2;
    QLabel* label;
    QLabel* label_2;
    QLabel* label_3;
    QLabel* label_4;
    QLabel* label_5;
    QLabel* label_6;
    QLabel* label_7;
    QLabel* label_8;
    QPushButton* pbOk;
    QSpacerItem* verticalSpacer;
    QVBoxLayout* verticalLayout;
    QVBoxLayout* verticalLayout_2;

    void setupUi(QWidget* Form); // setupUi
    void retranslateUi(QWidget* Form); // retranslateUi
    int dockWidgetFeature;
    // QWidget interface
protected:
    void showEvent(QShowEvent* event) override;
};

#endif // MATERIALSETUP_H
