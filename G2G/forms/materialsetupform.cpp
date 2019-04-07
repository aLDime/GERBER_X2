#include "materialsetupform.h"
#include "ui_materialsetupform.h"

#include <QDockWidget>
#include <QSettings>
#include <QTimer>
#include <mainwindow.h>
#include <scene.h>

MaterialSetup* MaterialSetup::self = nullptr;

QPointF MaterialSetup::homePos;
QPointF MaterialSetup::zeroPos;
double MaterialSetup::safeZ;
double MaterialSetup::thickness;
double MaterialSetup::clearence;
double MaterialSetup::plunge;

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

    connect(ui->dsbxHomeX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), MainWindow::self->home(), &Point::setPosX);
    connect(ui->dsbxHomeY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), MainWindow::self->home(), &Point::setPosY);
    connect(ui->dsbxZeroX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), MainWindow::self->zero(), &Point::setPosX);
    connect(ui->dsbxZeroY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), MainWindow::self->zero(), &Point::setPosY);

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
    homePos = settings.value("HomePoint").toPointF();
    zeroPos = settings.value("ZeroPoint").toPointF();
    ui->dsbxSafeZ->setValue(settings.value("dsbxSafeZ", 20).toDouble());
    ui->dsbxClearence->setValue(settings.value("dsbxClearence", 10).toDouble());
    ui->dsbxPlunge->setValue(settings.value("dsbxPlunge", 2).toDouble());
    ui->dsbxThickness->setValue(settings.value("dsbxThickness", 1).toDouble());
    settings.endGroup();

    ui->dsbxHomeX->setValue(homePos.x());
    ui->dsbxHomeY->setValue(homePos.y());

    ui->dsbxZeroX->setValue(zeroPos.x());
    ui->dsbxZeroY->setValue(zeroPos.y());

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
    self = this;
}

MaterialSetup::~MaterialSetup()
{
    self = nullptr;

    homePos = QPointF(ui->dsbxHomeX->value(), ui->dsbxHomeY->value());
    zeroPos = QPointF(ui->dsbxZeroX->value(), ui->dsbxZeroY->value());

    QSettings settings;
    settings.beginGroup("Material");
    settings.setValue("HomePoint", homePos);
    settings.setValue("ZeroPoint", zeroPos);
    settings.setValue("dsbxSafeZ", ui->dsbxSafeZ->value());
    settings.setValue("dsbxClearence", ui->dsbxClearence->value());
    settings.setValue("dsbxPlunge", ui->dsbxPlunge->value());
    settings.setValue("dsbxThickness", ui->dsbxThickness->value());
    settings.endGroup();

    safeZ = ui->dsbxSafeZ->value();
    thickness = ui->dsbxThickness->value();
    clearence = ui->dsbxClearence->value();
    plunge = ui->dsbxPlunge->value();

    delete ui;
}

void MaterialSetup::setHomePos(QPointF pos)
{
    QSettings settings;
    settings.beginGroup("Material");
    settings.setValue("HomePoint", pos);
    settings.endGroup();
    homePos = pos;
    ui->dsbxHomeX->setValue(pos.x());
    ui->dsbxHomeY->setValue(pos.y());
}

void MaterialSetup::setZeroPos(QPointF pos)
{
    QSettings settings;
    settings.beginGroup("Material");
    settings.setValue("ZeroPoint", pos);
    settings.endGroup();
    zeroPos = pos;
    ui->dsbxZeroX->setValue(pos.x());
    ui->dsbxZeroY->setValue(pos.y());
}
