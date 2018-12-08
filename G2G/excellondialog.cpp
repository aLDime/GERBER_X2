#include "excellondialog.h"
#include "ui_excellondialog.h"
#include <QWidget>

ExcellonDialog::ExcellonDialog(DrillFile* file)
    : m_file(file)
    , m_format(file->format())
    , m_tmpFormat(file->format())
    , ui(new Ui::ExcellonDialog)
{
    ui->setupUi(this);

    setWindowFlag(Qt::WindowStaysOnTopHint);

    ui->sbxInteger->setValue(m_format.integer);
    ui->sbxDecimal->setValue(m_format.decimal);

    ui->dsbxX->setValue(m_format.offsetPos.x());
    ui->dsbxY->setValue(m_format.offsetPos.y());

    m_format.unitMode == Inches ? ui->rbInches->setChecked(true) : ui->rbMillimeters->setChecked(true);
    m_format.zeroMode == LeadingZeros ? ui->rbLeading->setChecked(true) : ui->rbTrailing->setChecked(true);

    connect(ui->buttonBox, &QDialogButtonBox::rejected, [=] { m_file->setFormat(m_format); hide(); deleteLater(); });
    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=] { hide(); deleteLater(); });

    auto dsbxValueChanged = static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged);
    connect(ui->dsbxX, dsbxValueChanged, [&](double val) { m_tmpFormat.offsetPos.rx() = val; m_file->setFormat(m_tmpFormat); });
    connect(ui->dsbxY, dsbxValueChanged, [&](double val) { m_tmpFormat.offsetPos.ry() = val; m_file->setFormat(m_tmpFormat); });

    auto sbxValueChanged = static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged);
    connect(ui->sbxInteger, sbxValueChanged, [&](double val) { m_tmpFormat.integer = val; m_file->setFormat(m_tmpFormat); });
    connect(ui->sbxDecimal, sbxValueChanged, [&](double val) { m_tmpFormat.decimal = val; m_file->setFormat(m_tmpFormat); });

    connect(ui->rbInches, &QRadioButton::toggled, [&](bool checked) { m_tmpFormat.unitMode = static_cast<UnitMode>(checked), m_tmpFormat.zeroMode = static_cast<ZeroMode>(checked), m_file->setFormat(m_tmpFormat); });
    connect(ui->rbLeading, &QRadioButton::toggled, [&](bool checked) { m_tmpFormat.zeroMode = static_cast<ZeroMode>(checked), m_tmpFormat.unitMode = static_cast<UnitMode>(checked), m_file->setFormat(m_tmpFormat); });

    on_pbStep_clicked();
}

ExcellonDialog::~ExcellonDialog()
{
    qDebug("~ExcellonDialog()");
    delete ui;
}

void ExcellonDialog::on_pbStep_clicked()
{
    if (++m_step == 4)
        m_step = -1;
    const double singleStep = pow(0.1, m_step);
    ui->pbStep->setText("x" + QString::number(singleStep));
    ui->dsbxX->setSingleStep(singleStep);
    ui->dsbxY->setSingleStep(singleStep);
}

void ExcellonDialog::closeEvent(QCloseEvent* event)
{
    m_file->setFormat(m_format);
    deleteLater();
    QDialog::closeEvent(event);
}
