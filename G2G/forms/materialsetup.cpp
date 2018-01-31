#include "materialsetup.h"
#include "mainwindow.h"

#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QPushButton>
#include <QSpacerItem>
#include <QApplication>
#include <QDockWidget>
#include <QTimer>
#include <QSettings>

#include <graphicsview/mygraphicsscene.h>
#include <graphicsview/point.h>

bool MaterialSetup::isCreated;
MaterialSetup* MaterialSetup::t;

QPointF MaterialSetup::homePos;
QPointF MaterialSetup::zeroPos;
double MaterialSetup::z;
double MaterialSetup::thickness;
double MaterialSetup::clearence;
double MaterialSetup::plunge;

MaterialSetup::MaterialSetup(QWidget* parent)
    : QWidget(parent)
{
    t = this;
    setupUi(this);
    isCreated = true;

    connect(dsbxClearence, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double value) {
        if (value > dsbxHomeZ->value()) {
            dsbxHomeZ->setValue(value);
            dsbxZeroZ->setValue(value);
        }
        if (value < dsbxPlunge->value())
            dsbxPlunge->setValue(value);
    });

    connect(dsbxPlunge, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double value) {
        if (value > dsbxZeroZ->value()) {
            dsbxHomeZ->setValue(value);
            dsbxZeroZ->setValue(value);
        }
        if (value > dsbxClearence->value())
            dsbxClearence->setValue(value);
    });

    connect(dsbxHomeX, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double value) {
        MainWindow::getMainWindow()->getScene()->getItemHome()->setPos(value, dsbxHomeY->value());
    });
    connect(dsbxHomeY, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double value) {
        MainWindow::getMainWindow()->getScene()->getItemHome()->setPos(dsbxHomeX->value(), value);
    });
    connect(dsbxZeroX, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double value) {
        MainWindow::getMainWindow()->getScene()->getItemZero()->setPos(value, dsbxZeroY->value());
    });
    connect(dsbxZeroY, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double value) {
        MainWindow::getMainWindow()->getScene()->getItemZero()->setPos(dsbxZeroX->value(), value);
    });

    auto setZValue = [=](double value) {
        dsbxHomeZ->setValue(value);
        dsbxZeroZ->setValue(value);
        if (value < dsbxClearence->value())
            dsbxClearence->setValue(value);
        if (value < dsbxPlunge->value())
            dsbxPlunge->setValue(value);
    };
    connect(dsbxHomeZ, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), setZValue);
    connect(dsbxZeroZ, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), setZValue);

    QSettings settings;
    settings.beginGroup("Material");

    QPointF ph(settings.value("HomeXY").toPointF());
    dsbxHomeX->setValue(ph.x());
    dsbxHomeY->setValue(ph.y());
    QPointF pz(settings.value("ZeroXY").toPointF());
    dsbxZeroX->setValue(pz.x());
    dsbxZeroY->setValue(pz.y());

    dsbxZeroZ->setValue(settings.value("HomeZ", 20).toDouble());
    dsbxClearence->setValue(settings.value("Clearenc", 10).toDouble());
    dsbxPlunge->setValue(settings.value("Plunge", 2).toDouble());
    dsbxThickness->setValue(settings.value("Thickness", 1).toDouble());

    settings.endGroup();

    homePos = ph;
    zeroPos = pz;
    z = dsbxHomeZ->value();
    thickness = dsbxThickness->value();
    clearence = dsbxClearence->value();
    plunge = dsbxPlunge->value();
}

MaterialSetup::~MaterialSetup()
{
    isCreated = false;
    QSettings settings;
    settings.beginGroup("Material");
    settings.setValue("HomeXY", QPointF(dsbxHomeX->value(), dsbxHomeY->value()));
    settings.setValue("ZeroXY", QPointF(dsbxZeroX->value(), dsbxZeroY->value()));
    settings.setValue("HomeZ", dsbxZeroZ->value());
    settings.setValue("Clearenc", dsbxClearence->value());
    settings.setValue("Plunge", dsbxPlunge->value());
    settings.endGroup();

    homePos = QPointF(dsbxHomeX->value(), dsbxHomeY->value());
    zeroPos = QPointF(dsbxZeroX->value(), dsbxZeroY->value());
    z = dsbxHomeZ->value();
    thickness = dsbxThickness->value();
    clearence = dsbxClearence->value();
    plunge = dsbxPlunge->value();
}

MaterialSetup* MaterialSetup::This() { return t; }

void MaterialSetup::setHomePos(QPointF pos)
{
    if (isCreated) {
        dsbxHomeX->setValue(pos.x());
        dsbxHomeY->setValue(pos.y());
    }
}

void MaterialSetup::setZeroPos(QPointF pos)
{
    if (isCreated) {
        dsbxZeroX->setValue(pos.x());
        dsbxZeroY->setValue(pos.y());
    }
}

void MaterialSetup::setupUi(QWidget* Form)
{
    if (Form->objectName().isEmpty())
        Form->setObjectName(QStringLiteral("Form"));
    verticalLayout = new QVBoxLayout(Form);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    verticalLayout->setContentsMargins(6, 6, 6, 6);
    groupBox = new QGroupBox(Form);
    groupBox->setObjectName(QStringLiteral("groupBox"));
    verticalLayout_2 = new QVBoxLayout(groupBox);
    verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
    verticalLayout_2->setContentsMargins(6, 6, 6, 6);
    label = new QLabel(groupBox);
    label->setObjectName(QStringLiteral("label"));

    verticalLayout_2->addWidget(label);

    verticalLayout->addWidget(groupBox);

    groupBox_2 = new QGroupBox(Form);
    groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
    label_3 = new QLabel(groupBox_2);
    label_3->setObjectName(QStringLiteral("label_3"));

    label_2 = new QLabel(groupBox_2);
    label_2->setObjectName(QStringLiteral("label_2"));

    dsbxThickness = new QDoubleSpinBox(groupBox_2);
    dsbxThickness->setObjectName(QStringLiteral("dsbxThickness"));
    dsbxThickness->setButtonSymbols(QAbstractSpinBox::NoButtons);
    dsbxThickness->setSingleStep(0.1);

    line = new QFrame(groupBox_2);
    line->setObjectName(QStringLiteral("line"));
    line->setFrameShadow(QFrame::Plain);
    line->setFrameShape(QFrame::HLine);

    label_4 = new QLabel(groupBox_2);
    label_4->setObjectName(QStringLiteral("label_4"));

    label_5 = new QLabel(groupBox_2);
    label_5->setObjectName(QStringLiteral("label_5"));

    dsbxClearence = new QDoubleSpinBox(groupBox_2);
    dsbxClearence->setObjectName(QStringLiteral("dsbxClearence"));
    dsbxClearence->setButtonSymbols(QAbstractSpinBox::NoButtons);

    label_6 = new QLabel(groupBox_2);
    label_6->setObjectName(QStringLiteral("label_6"));

    dsbxPlunge = new QDoubleSpinBox(groupBox_2);
    dsbxPlunge->setObjectName(QStringLiteral("dsbxPlunge"));
    dsbxPlunge->setButtonSymbols(QAbstractSpinBox::NoButtons);

    line_2 = new QFrame(groupBox_2);
    line_2->setObjectName(QStringLiteral("line_2"));
    line_2->setFrameShadow(QFrame::Plain);
    line_2->setFrameShape(QFrame::HLine);

    label_7 = new QLabel(groupBox_2);
    label_7->setObjectName(QStringLiteral("label_7"));

    int range = 1000;
    dsbxHomeX = new QDoubleSpinBox(groupBox_2);
    dsbxHomeX->setObjectName(QStringLiteral("dsbxHomeX"));
    dsbxHomeX->setButtonSymbols(QAbstractSpinBox::NoButtons);
    dsbxHomeX->setRange(-range, range);

    dsbxHomeY = new QDoubleSpinBox(groupBox_2);
    dsbxHomeY->setObjectName(QStringLiteral("dsbxHomeY"));
    dsbxHomeY->setButtonSymbols(QAbstractSpinBox::NoButtons);
    dsbxHomeY->setRange(-range, range);

    dsbxHomeZ = new QDoubleSpinBox(groupBox_2);
    dsbxHomeZ->setObjectName(QStringLiteral("dsbxHomeZ"));
    dsbxHomeZ->setButtonSymbols(QAbstractSpinBox::NoButtons);
    dsbxHomeZ->setMaximum(100);

    line_3 = new QFrame(groupBox_2);
    line_3->setObjectName(QStringLiteral("line_3"));
    line_3->setFrameShadow(QFrame::Plain);
    line_3->setFrameShape(QFrame::HLine);

    label_8 = new QLabel(groupBox_2);
    label_8->setObjectName(QStringLiteral("label_8"));

    dsbxZeroX = new QDoubleSpinBox(groupBox_2);
    dsbxZeroX->setObjectName(QStringLiteral("dsbxZeroX"));
    dsbxZeroX->setButtonSymbols(QAbstractSpinBox::NoButtons);
    dsbxZeroX->setRange(-range, range);

    dsbxZeroY = new QDoubleSpinBox(groupBox_2);
    dsbxZeroY->setObjectName(QStringLiteral("dsbxZeroY"));
    dsbxZeroY->setButtonSymbols(QAbstractSpinBox::NoButtons);
    dsbxZeroY->setRange(-range, range);

    dsbxZeroZ = new QDoubleSpinBox(groupBox_2);
    dsbxZeroZ->setObjectName(QStringLiteral("dsbxZeroY"));
    dsbxZeroZ->setButtonSymbols(QAbstractSpinBox::NoButtons);
    dsbxZeroZ->setMaximum(100);

    formLayout = new QFormLayout(groupBox_2);
    formLayout->setObjectName(QStringLiteral("formLayout"));
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    formLayout->setContentsMargins(6, 6, 6, 6);
    formLayout->setWidget(0, QFormLayout::SpanningRole, label_3);
    formLayout->setWidget(1, QFormLayout::LabelRole, label_2);
    formLayout->setWidget(1, QFormLayout::FieldRole, dsbxThickness);
    formLayout->setWidget(2, QFormLayout::SpanningRole, line);
    formLayout->setWidget(3, QFormLayout::SpanningRole, label_4);
    formLayout->setWidget(4, QFormLayout::LabelRole, label_5);
    formLayout->setWidget(4, QFormLayout::FieldRole, dsbxClearence);
    formLayout->setWidget(5, QFormLayout::LabelRole, label_6);
    formLayout->setWidget(5, QFormLayout::FieldRole, dsbxPlunge);
    formLayout->setWidget(6, QFormLayout::SpanningRole, line_2);
    formLayout->setWidget(7, QFormLayout::SpanningRole, label_7);
    formLayout->setWidget(8, QFormLayout::LabelRole, new QLabel("Z:", groupBox_2));
    formLayout->setWidget(8, QFormLayout::FieldRole, dsbxHomeX);
    formLayout->setWidget(9, QFormLayout::LabelRole, new QLabel("Y:", groupBox_2));
    formLayout->setWidget(9, QFormLayout::FieldRole, dsbxHomeY);
    formLayout->setWidget(10, QFormLayout::LabelRole, new QLabel("Z:", groupBox_2));
    formLayout->setWidget(10, QFormLayout::FieldRole, dsbxHomeZ);
    formLayout->setWidget(11, QFormLayout::SpanningRole, line_3);
    formLayout->setWidget(12, QFormLayout::SpanningRole, label_8);
    formLayout->setWidget(13, QFormLayout::LabelRole, new QLabel("X:", groupBox_2));
    formLayout->setWidget(13, QFormLayout::FieldRole, dsbxZeroX);
    formLayout->setWidget(14, QFormLayout::LabelRole, new QLabel("Y:", groupBox_2));
    formLayout->setWidget(14, QFormLayout::FieldRole, dsbxZeroY);
    formLayout->setWidget(15, QFormLayout::LabelRole, new QLabel("Z:", groupBox_2));
    formLayout->setWidget(15, QFormLayout::FieldRole, dsbxZeroZ);

    verticalLayout->addWidget(groupBox_2);

    pbOk = new QPushButton(Form);
    pbOk->setObjectName(QStringLiteral("bpOk"));

    verticalLayout->addWidget(pbOk);

    verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout->addItem(verticalSpacer);

    QLatin1String styleSheet("QGroupBox {"
                             "background-color: rgb(255,255,255);"
                             "border: 1px solid gray;"
                             "border-radius: 5px;"
                             "margin-top: 1ex; /* leave space at the top for the title */"
                             "}"
                             "QGroupBox::title {"
                             "subcontrol-origin: margin;"
                             "margin-top: -2ex;"
                             "subcontrol-position: top center; /* position at the top center */"
                             "padding: 0 6px;"
                             "}");
    groupBox->setStyleSheet(styleSheet);
    groupBox_2->setStyleSheet(styleSheet);

    connect(pbOk, &QPushButton::clicked, [=] {
        if (parent()
            && dsbxThickness->value() > 0.0
            && dsbxClearence->value() > 0.0
            && dsbxHomeZ->value() > 0.0
            && dsbxZeroZ->value() > 0.0) {
            static_cast<QDockWidget*>(parent())->hide();
            return;
        }
        QString s("QDoubleSpinBox{background: red;}");
        if (dsbxThickness->value() == 0.0) {
            int t = 1, tt = 100;
            QTimer::singleShot(tt * t++, [=] { dsbxThickness->setStyleSheet(s); });
            QTimer::singleShot(tt * t++, [=] { dsbxThickness->setStyleSheet(""); });
            QTimer::singleShot(tt * t++, [=] { dsbxThickness->setStyleSheet(s); });
            QTimer::singleShot(tt * t++, [=] { dsbxThickness->setStyleSheet(""); });
            QTimer::singleShot(tt * t++, [=] { dsbxThickness->setStyleSheet(s); });
            QTimer::singleShot(tt * t++, [=] { dsbxThickness->setStyleSheet(""); });
        }
        if (dsbxClearence->value() == 0.0) {
            int t = 1, tt = 100;
            QTimer::singleShot(tt * t++, [=] { dsbxClearence->setStyleSheet(s); });
            QTimer::singleShot(tt * t++, [=] { dsbxClearence->setStyleSheet(""); });
            QTimer::singleShot(tt * t++, [=] { dsbxClearence->setStyleSheet(s); });
            QTimer::singleShot(tt * t++, [=] { dsbxClearence->setStyleSheet(""); });
            QTimer::singleShot(tt * t++, [=] { dsbxClearence->setStyleSheet(s); });
            QTimer::singleShot(tt * t++, [=] { dsbxClearence->setStyleSheet(""); });
        }
        if (dsbxHomeZ->value() == 0.0) {
            int t = 1, tt = 100;
            QTimer::singleShot(tt * t++, [=] { dsbxHomeZ->setStyleSheet(s); });
            QTimer::singleShot(tt * t++, [=] { dsbxHomeZ->setStyleSheet(""); });
            QTimer::singleShot(tt * t++, [=] { dsbxHomeZ->setStyleSheet(s); });
            QTimer::singleShot(tt * t++, [=] { dsbxHomeZ->setStyleSheet(""); });
            QTimer::singleShot(tt * t++, [=] { dsbxHomeZ->setStyleSheet(s); });
            QTimer::singleShot(tt * t++, [=] { dsbxHomeZ->setStyleSheet(""); });
        }
        if (dsbxZeroZ->value() == 0.0) {
            int t = 1, tt = 100;
            QTimer::singleShot(tt * t++, [=] { dsbxZeroZ->setStyleSheet(s); });
            QTimer::singleShot(tt * t++, [=] { dsbxZeroZ->setStyleSheet(""); });
            QTimer::singleShot(tt * t++, [=] { dsbxZeroZ->setStyleSheet(s); });
            QTimer::singleShot(tt * t++, [=] { dsbxZeroZ->setStyleSheet(""); });
            QTimer::singleShot(tt * t++, [=] { dsbxZeroZ->setStyleSheet(s); });
            QTimer::singleShot(tt * t++, [=] { dsbxZeroZ->setStyleSheet(""); });
        }
    });

    retranslateUi(Form);

    QMetaObject::connectSlotsByName(Form);
}

void MaterialSetup::retranslateUi(QWidget* Form)
{
    Form->setWindowTitle(QApplication::translate("Form", "Material Setup", Q_NULLPTR));
    dsbxClearence->setSuffix(QApplication::translate("Form", " mm", Q_NULLPTR));
    dsbxHomeX->setSuffix(QApplication::translate("Form", " mm", Q_NULLPTR));
    dsbxHomeY->setSuffix(QApplication::translate("Form", " mm", Q_NULLPTR));
    dsbxHomeZ->setSuffix(QApplication::translate("Form", " mm", Q_NULLPTR));
    dsbxPlunge->setSuffix(QApplication::translate("Form", " mm", Q_NULLPTR));
    dsbxThickness->setSuffix(QApplication::translate("Form", " mm", Q_NULLPTR));
    dsbxZeroX->setSuffix(QApplication::translate("Form", " mm", Q_NULLPTR));
    dsbxZeroY->setSuffix(QApplication::translate("Form", " mm", Q_NULLPTR));
    dsbxZeroZ->setSuffix(QApplication::translate("Form", " mm", Q_NULLPTR));
    label->setText(QApplication::translate("Form", "<html><head/><body><p><span style=\" font-weight:600;\">Material Setup</span></p></body></html>", Q_NULLPTR));
    label_2->setText(QApplication::translate("Form", "Thickness:", Q_NULLPTR));
    label_3->setText(QApplication::translate("Form", "<html><head/><body><p><span style=\" font-weight:600;\">Material</span></p></body></html>", Q_NULLPTR));
    label_4->setText(QApplication::translate("Form", "<html><head/><body><p><span style=\" font-weight:600;\">Rapid z gaps above Material</span></p></body></html>", Q_NULLPTR));
    label_5->setText(QApplication::translate("Form", "Clearence:", Q_NULLPTR));
    label_6->setText(QApplication::translate("Form", "Plunge:", Q_NULLPTR));
    label_7->setText(QApplication::translate("Form", "<html><head/><body><p><span style=\" font-weight:600;\">Home Position</span></p></body></html>", Q_NULLPTR));
    label_8->setText(QApplication::translate("Form", "<html><head/><body><p><span style=\" font-weight:600;\">Zero Position</span></p></body></html>", Q_NULLPTR));
    pbOk->setText(QApplication::translate("Form", "Ok", Q_NULLPTR));
}

void MaterialSetup::showEvent(QShowEvent* event)
{
    QDockWidget* w = static_cast<QDockWidget*>(parent());
    dockWidgetFeature = static_cast<int>(w->features());
    int tmp = static_cast<int>(w->features()) & ~1;
    w->setFeatures(static_cast<QDockWidget::DockWidgetFeature>(tmp));
}
