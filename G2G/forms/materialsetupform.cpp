#include "materialsetupform.h"
#include "ui_materialsetupform.h"

#include <QDockWidget>
#include <QSettings>
#include <QTimer>
#include <myscene.h>

MaterialSetupForm* MaterialSetupForm::self = nullptr;

QPointF MaterialSetupForm::homePos;
QPointF MaterialSetupForm::zeroPos;
double MaterialSetupForm::z;
double MaterialSetupForm::thickness;
double MaterialSetupForm::clearence;
double MaterialSetupForm::plunge;

MaterialSetupForm::MaterialSetupForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::MaterialSetupForm)

{
    self = this;
    ui->setupUi(this);

    auto dsbxValueChanged = static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged);

    connect(ui->dsbxClearence, dsbxValueChanged, [=](double value) {
        if (value > ui->dsbxSafeZ->value())
            ui->dsbxSafeZ->setValue(value);
        if (value < ui->dsbxPlunge->value())
            ui->dsbxPlunge->setValue(value);
    });

    connect(ui->dsbxPlunge, dsbxValueChanged, [=](double value) {
        if (value > ui->dsbxSafeZ->value())
            ui->dsbxSafeZ->setValue(value);
        if (value > ui->dsbxClearence->value())
            ui->dsbxClearence->setValue(value);
    });

    connect(ui->dsbxHomeX, dsbxValueChanged, [=](double value) { MyScene::self->getItemHome()->setPos(value, ui->dsbxHomeY->value()); });
    connect(ui->dsbxHomeY, dsbxValueChanged, [=](double value) { MyScene::self->getItemHome()->setPos(ui->dsbxHomeX->value(), value); });
    connect(ui->dsbxZeroX, dsbxValueChanged, [=](double value) { MyScene::self->getItemZero()->setPos(value, ui->dsbxZeroY->value()); });
    connect(ui->dsbxZeroY, dsbxValueChanged, [=](double value) { MyScene::self->getItemZero()->setPos(ui->dsbxZeroX->value(), value); });

    connect(ui->dsbxSafeZ, dsbxValueChanged, [=](double value) {
        ui->dsbxSafeZ->setValue(value);
        ui->dsbxSafeZ->setValue(value);
        if (value < ui->dsbxClearence->value())
            ui->dsbxClearence->setValue(value);
        if (value < ui->dsbxPlunge->value())
            ui->dsbxPlunge->setValue(value);
    });

    QSettings settings;
    settings.beginGroup("Material");

    QPointF point(settings.value("HomePoint").toPointF());
    ui->dsbxHomeX->setValue(point.x());
    ui->dsbxHomeY->setValue(point.y());
    homePos = point;

    point = settings.value("ZeroPoint").toPointF();
    ui->dsbxZeroX->setValue(point.x());
    ui->dsbxZeroY->setValue(point.y());
    zeroPos = point;

    ui->dsbxSafeZ->setValue(settings.value("SafeZ", 20).toDouble());
    ui->dsbxClearence->setValue(settings.value("Clearenc", 10).toDouble());
    ui->dsbxPlunge->setValue(settings.value("Plunge", 2).toDouble());
    ui->dsbxThickness->setValue(settings.value("Thickness", 1).toDouble());

    settings.endGroup();

    z = ui->dsbxSafeZ->value();
    thickness = ui->dsbxThickness->value();
    clearence = ui->dsbxClearence->value();
    plunge = ui->dsbxPlunge->value();

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
    //ui->groupBox->setStyleSheet(styleSheet);
    //ui->groupBox_2->setStyleSheet(styleSheet);

    connect(ui->pbOk, &QPushButton::clicked, [=] {
        if (this->parent()
            && ui->dsbxThickness->value() > 0.0
            && ui->dsbxClearence->value() > 0.0
            && ui->dsbxSafeZ->value() > 0.0) {
            static_cast<QDockWidget*>(this->parent())->hide();
            return;
        }
        QString s("QDoubleSpinBox{background: red;}");
        if (ui->dsbxThickness->value() == 0.0) {
            int t = 1, tt = 100;
            QTimer::singleShot(tt * t++, [=] { ui->dsbxThickness->setStyleSheet(s); });
            QTimer::singleShot(tt * t++, [=] { ui->dsbxThickness->setStyleSheet(""); });
            QTimer::singleShot(tt * t++, [=] { ui->dsbxThickness->setStyleSheet(s); });
            QTimer::singleShot(tt * t++, [=] { ui->dsbxThickness->setStyleSheet(""); });
            QTimer::singleShot(tt * t++, [=] { ui->dsbxThickness->setStyleSheet(s); });
            QTimer::singleShot(tt * t++, [=] { ui->dsbxThickness->setStyleSheet(""); });
        }
        if (ui->dsbxClearence->value() == 0.0) {
            int t = 1, tt = 100;
            QTimer::singleShot(tt * t++, [=] { ui->dsbxClearence->setStyleSheet(s); });
            QTimer::singleShot(tt * t++, [=] { ui->dsbxClearence->setStyleSheet(""); });
            QTimer::singleShot(tt * t++, [=] { ui->dsbxClearence->setStyleSheet(s); });
            QTimer::singleShot(tt * t++, [=] { ui->dsbxClearence->setStyleSheet(""); });
            QTimer::singleShot(tt * t++, [=] { ui->dsbxClearence->setStyleSheet(s); });
            QTimer::singleShot(tt * t++, [=] { ui->dsbxClearence->setStyleSheet(""); });
        }
    });
}

MaterialSetupForm::~MaterialSetupForm()
{
    self = nullptr;

    QSettings settings;
    settings.beginGroup("Material");
    settings.setValue("HomePoint", QPointF(ui->dsbxHomeX->value(), ui->dsbxHomeY->value()));
    settings.setValue("ZeroPoint", QPointF(ui->dsbxZeroX->value(), ui->dsbxZeroY->value()));
    settings.setValue("SafeZ", ui->dsbxSafeZ->value());
    settings.setValue("Clearenc", ui->dsbxClearence->value());
    settings.setValue("Plunge", ui->dsbxPlunge->value());
    settings.endGroup();

    homePos = QPointF(ui->dsbxHomeX->value(), ui->dsbxHomeY->value());
    zeroPos = QPointF(ui->dsbxZeroX->value(), ui->dsbxZeroY->value());
    z = ui->dsbxSafeZ->value();
    thickness = ui->dsbxThickness->value();
    clearence = ui->dsbxClearence->value();
    plunge = ui->dsbxPlunge->value();

    delete ui;
}

void MaterialSetupForm::setHomePos(QPointF pos)
{
    QSettings settings;
    settings.beginGroup("Material");
    settings.setValue("HomePoint", pos);
    settings.endGroup();
    homePos = pos;
    if (self != nullptr) {
        ui->dsbxHomeX->setValue(pos.x());
        ui->dsbxHomeY->setValue(pos.y());
    }
}

void MaterialSetupForm::setZeroPos(QPointF pos)
{
    QSettings settings;
    settings.beginGroup("Material");
    settings.setValue("ZeroPoint", pos);
    settings.endGroup();
    zeroPos = pos;
    if (self != nullptr) {
        ui->dsbxZeroX->setValue(pos.x());
        ui->dsbxZeroY->setValue(pos.y());
    }
}
