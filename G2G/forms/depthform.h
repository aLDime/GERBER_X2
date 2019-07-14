#ifndef DEPTHFORM_H
#define DEPTHFORM_H

#include <QWidget>

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QWidget>

#include "gcodepropertiesform.h"

class DepthForm : public QWidget {
    Q_OBJECT
public:
    explicit DepthForm(QWidget* parent = nullptr);

    double value() const;
    void setValue(double value);
    QRadioButton* rbCopper;
    QRadioButton* rbBoard;
    QRadioButton* rbCustom;
signals:
    void valueChanged(double);

private:
    QHBoxLayout* horizontalLayout;
    QDoubleSpinBox* dsbx;
    void setupUi(QWidget* Form); // setupUi
    void retranslateUi(QWidget* Form); // retranslateUi
};

//class DepthForm : public QWidget
//{
//    Q_OBJECT
//public:
//    explicit DepthForm(QWidget *parent = nullptr);

//signals:

//public slots:
//};

#endif // DEPTHFORM_H
