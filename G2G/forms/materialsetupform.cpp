#include "materialsetupform.h"
#include "ui_materialsetupform.h"

#include <QDockWidget>
#include <QSettings>
#include <QTimer>
#include <mainwindow.h>
#include <scene.h>

MaterialSetup* MaterialSetup::self = nullptr;

Point* MaterialSetup::homePoint = nullptr;
Point* MaterialSetup::zeroPoint = nullptr;
double MaterialSetup::safeZ;
double MaterialSetup::thickness;
double MaterialSetup::clearence;
double MaterialSetup::plunge;
double MaterialSetup::glue;

MaterialSetup::MaterialSetup(QWidget* prnt)
    : QWidget(prnt)
    , ui(new Ui::MaterialSetupForm)
{
    ui->setupUi(this);

    connect(ui->dsbxClearence, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        if (value > ui->dsbxSafeZ->value())
            ui->dsbxSafeZ->setValue(value);
        if (value < ui->dsbxPlunge->value())
            ui->dsbxPlunge->setValue(value);
    });

    connect(ui->dsbxPlunge, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        if (value > ui->dsbxSafeZ->value())
            ui->dsbxSafeZ->setValue(value);
        if (value > ui->dsbxClearence->value())
            ui->dsbxClearence->setValue(value);
    });

    connect(ui->dsbxGlue, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double val) { glue = val; });

    connect(ui->dsbxHomeX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double val) { homePoint->setPosX(val); });

    connect(ui->dsbxHomeY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double val) { homePoint->setPosY(val); });

    connect(ui->dsbxZeroX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double val) { zeroPoint->setPosX(val); });

    connect(ui->dsbxZeroY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double val) { zeroPoint->setPosY(val); });

    connect(ui->dsbxSafeZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        ui->dsbxSafeZ->setValue(value);
        ui->dsbxSafeZ->setValue(value);
        if (value < ui->dsbxClearence->value())
            ui->dsbxClearence->setValue(value);
        if (value < ui->dsbxPlunge->value())
            ui->dsbxPlunge->setValue(value);
    });

    QSettings settings;
    settings.beginGroup("Material");
    ui->dsbxSafeZ->setValue(settings.value("dsbxSafeZ", 20).toDouble());
    ui->dsbxClearence->setValue(settings.value("dsbxClearence", 10).toDouble());
    ui->dsbxPlunge->setValue(settings.value("dsbxPlunge", 2).toDouble());
    ui->dsbxThickness->setValue(settings.value("dsbxThickness", 1).toDouble());
    ui->dsbxGlue->setValue(settings.value("dsbxGlue", 0.01).toDouble());
    settings.endGroup();

    ui->dsbxHomeX->setValue(homePoint->pos().x());
    ui->dsbxHomeY->setValue(homePoint->pos().y());

    ui->dsbxZeroX->setValue(zeroPoint->pos().x());
    ui->dsbxZeroY->setValue(zeroPoint->pos().y());

    safeZ = ui->dsbxSafeZ->value();
    thickness = ui->dsbxThickness->value();
    clearence = ui->dsbxClearence->value();
    plunge = ui->dsbxPlunge->value();

    connect(ui->pbOk, &QPushButton::clicked, [=] {
        if (this->parent()
            && ui->dsbxThickness->value() > 0.0
            && ui->dsbxClearence->value() > 0.0
            && ui->dsbxSafeZ->value() > 0.0) {
            static_cast<QWidget*>(parent())->close();
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
    prnt->setWindowTitle(ui->label->text());
    self = this;

    for (QPushButton* button : findChildren<QPushButton*>()) {
        button->setIconSize({16,16});
    }
}

MaterialSetup::~MaterialSetup()
{
    self = nullptr;

    homePoint->setPos(QPointF(ui->dsbxHomeX->value(), ui->dsbxHomeY->value()));
    zeroPoint->setPos(QPointF(ui->dsbxZeroX->value(), ui->dsbxZeroY->value()));

    QSettings settings;
    settings.beginGroup("Material");
    settings.setValue("dsbxSafeZ", ui->dsbxSafeZ->value());
    settings.setValue("dsbxClearence", ui->dsbxClearence->value());
    settings.setValue("dsbxPlunge", ui->dsbxPlunge->value());
    settings.setValue("dsbxThickness", ui->dsbxThickness->value());
    settings.setValue("dsbxGlue", ui->dsbxGlue->value());
    settings.endGroup();

    safeZ = ui->dsbxSafeZ->value();
    thickness = ui->dsbxThickness->value();
    clearence = ui->dsbxClearence->value();
    plunge = ui->dsbxPlunge->value();

    delete ui;
}

void MaterialSetup::updatePosDsbxs()
{
    ui->dsbxHomeX->setValue(homePoint->pos().x());
    ui->dsbxHomeY->setValue(homePoint->pos().y());
    ui->dsbxZeroX->setValue(zeroPoint->pos().x());
    ui->dsbxZeroY->setValue(zeroPoint->pos().y());
}
