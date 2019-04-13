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

    ui->rbInches->setChecked(!m_format.unitMode);
    ui->rbMillimeters->setChecked(m_format.unitMode);

    ui->rbLeading->setChecked(!m_format.zeroMode);
    ui->rbTrailing->setChecked(m_format.zeroMode);

    connect(ui->buttonBox, &QDialogButtonBox::rejected, [=] { m_file->setFormat(m_format); hide(); deleteLater(); });
    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=] { hide(); deleteLater(); });

    connect(ui->dsbxX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&] { updateFormat(); });
    connect(ui->dsbxY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&] { updateFormat(); });

    connect(ui->sbxInteger, QOverload<int>::of(&QSpinBox::valueChanged), [&] { updateFormat(); });
    connect(ui->sbxDecimal, QOverload<int>::of(&QSpinBox::valueChanged), [&] { updateFormat(); });

    connect(ui->rbInches, &QRadioButton::toggled, [&] { updateFormat(); });
    connect(ui->rbLeading, &QRadioButton::toggled, [&] { updateFormat(); });
    connect(ui->rbMillimeters, &QRadioButton::toggled, [&] { updateFormat(); });
    connect(ui->rbTrailing, &QRadioButton::toggled, [&] { updateFormat(); });

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

void ExcellonDialog::updateFormat()
{
    m_tmpFormat.offsetPos.rx() = ui->dsbxX->value();
    m_tmpFormat.offsetPos.ry() = ui->dsbxY->value();

    m_tmpFormat.integer = ui->sbxInteger->value();
    m_tmpFormat.decimal = ui->sbxDecimal->value();

    m_tmpFormat.unitMode = static_cast<UnitMode>(ui->rbMillimeters->isChecked());
    m_tmpFormat.zeroMode = static_cast<ZeroMode>(ui->rbTrailing->isChecked());

    m_file->setFormat(m_tmpFormat);
}

void ExcellonDialog::closeEvent(QCloseEvent* event)
{
    m_file->setFormat(m_format);
    deleteLater();
    QDialog::closeEvent(event);
}
