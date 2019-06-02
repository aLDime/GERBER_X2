#include "excellondialog.h"
#include "exfile.h"
#include "ui_excellondialog.h"
#include <cmath> // pow()
#include <graphicsview.h>
#include <scene.h>

using namespace Excellon;

ExcellonDialog::ExcellonDialog(Excellon::File* file)
    : m_file(file)
    , m_format(file->format())
    , m_tmpFormat(file->format())
    , ui(new Ui::ExcellonDialog)
{
    ui->setupUi(this);
    setObjectName("ExcellonDialog");

    setWindowFlag(Qt::WindowStaysOnTopHint);

    ui->sbxInteger->setValue(m_format.integer);
    ui->sbxDecimal->setValue(m_format.decimal);

    ui->dsbxX->setValue(m_format.offsetPos.x());
    ui->dsbxY->setValue(m_format.offsetPos.y());

    ui->rbInches->setChecked(!m_format.unitMode);
    ui->rbMillimeters->setChecked(m_format.unitMode);

    ui->rbLeading->setChecked(!m_format.zeroMode);
    ui->rbTrailing->setChecked(m_format.zeroMode);

    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ExcellonDialog::rejectFormat);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ExcellonDialog::acceptFormat);

    connect(ui->dsbxX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double val) { m_tmpFormat.offsetPos.setX(val); m_file->setFormat(m_tmpFormat); });
    connect(ui->dsbxY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double val) { m_tmpFormat.offsetPos.setY(val); m_file->setFormat(m_tmpFormat); });

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
    qDebug("updateFormat");
    m_tmpFormat.offsetPos.rx() = ui->dsbxX->value();
    m_tmpFormat.offsetPos.ry() = ui->dsbxY->value();

    m_tmpFormat.integer = ui->sbxInteger->value();
    m_tmpFormat.decimal = ui->sbxDecimal->value();

    m_tmpFormat.unitMode = static_cast<UnitMode>(ui->rbMillimeters->isChecked());
    m_tmpFormat.zeroMode = static_cast<ZeroMode>(ui->rbTrailing->isChecked());

    m_file->setFormat(m_tmpFormat);
    GraphicsView::self->zoomFit();
}

void ExcellonDialog::acceptFormat()
{
    GraphicsView::self->zoomFit();
    deleteLater();
}

void ExcellonDialog::rejectFormat()
{
    GraphicsView::self->zoomFit();
    //if (Project::contains(m_file))
    m_file->setFormat(m_format);
    deleteLater();
}

void ExcellonDialog::closeEvent(QCloseEvent* event)
{
    //if (Project::contains(m_file))
    m_file->setFormat(m_format);
    deleteLater();
    QDialog::closeEvent(event);
}

void ExcellonDialog::on_pushButton_clicked()
{
    QPair<QPointF, QPointF> pair;
    int c = 0;
    for (QGraphicsItem* item : GraphicsView::self->scene()->selectedItems()) {
        if (item->type() == DrillItemType) {
            pair.first = item->boundingRect().center();
            ++c;
        }
        if (item->type() != DrillItemType) {
            pair.second = item->boundingRect().center();
            ++c;
        }
        if (c == 2) {
            QPointF p(pair.second - (pair.first - m_tmpFormat.offsetPos));
            if (QLineF(pair.first, pair.second).length() < 0.001) // 1 uMetr
                return;
            ui->dsbxX->setValue(p.x());
            ui->dsbxY->setValue(p.y());
            GraphicsView::self->zoomFit();
            return;
        }
    }
}
