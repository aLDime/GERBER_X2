#include "tooleditform.h"
#include "toolitem.h"
#include "toolmodel.h"
#include "ui_tooleditform.h"

#include <QDebug>
#include <QPicture>
#include <QSettings>
#include <QTimer>

ToolEditForm::ToolEditForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ToolEditForm)
{
    ui->setupUi(this);

    const auto dsbxValueChanged = static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged);
    connect(ui->dsbxAngle, dsbxValueChanged, this, &ToolEditForm::valueChanged);
    connect(ui->dsbxDiameter, dsbxValueChanged, this, &ToolEditForm::valueChanged);
    connect(ui->dsbxFeedRate, dsbxValueChanged, this, &ToolEditForm::valueChanged);
    connect(ui->dsbxOneTurnCut, dsbxValueChanged, this, &ToolEditForm::valueChanged);
    connect(ui->dsbxOneTurnCutPercent, dsbxValueChanged, this, &ToolEditForm::valueChanged);
    connect(ui->dsbxPassDepth, dsbxValueChanged, this, &ToolEditForm::valueChanged);
    connect(ui->dsbxPlungeRate, dsbxValueChanged, this, &ToolEditForm::valueChanged);
    connect(ui->dsbxSpindleSpeed, dsbxValueChanged, this, &ToolEditForm::valueChanged);
    connect(ui->dsbxStepover, dsbxValueChanged, this, &ToolEditForm::valueChanged);
    connect(ui->dsbxStepoverPercent, dsbxValueChanged, this, &ToolEditForm::valueChanged);

    ui->cbxUnits->setVisible(false);

    QSettings settings;
    ui->cbxFeedSpeeds->setCurrentIndex(settings.value("cbxFeedSpeeds").toInt());
    //ui->cbxToolType->setCurrentIndex(settings.value("cbxToolType").toInt());
    //ui->cbxUnits->setCurrentIndex(settings.value("cbxUnits").toInt());

    setVisibleWidgets(false);
    ui->pbApply->setStyleSheet("");
}

ToolEditForm::~ToolEditForm()
{
    QSettings settings;
    settings.setValue("cbxFeedSpeeds", ui->cbxFeedSpeeds->currentIndex());
    //settings.setValue("cbxToolType", ui->cbxToolType->currentIndex());
    //settings.setValue("cbxUnits", ui->cbxUnits->currentIndex());

    delete ui;
}

void ToolEditForm::setItem(ToolItem* item)
{
    if (item == nullptr)
        return;

    m_item = item;
    if (m_item->isTool()) {
        setTool(item->tool());
    } else {
        ui->leName->setText(m_item->name());
        ui->teNote->setText(m_item->note());
        setVisibleWidgets(false);
    }
    ui->pbApply->setStyleSheet("");
}

void ToolEditForm::setRed()
{
    ui->pbApply->setStyleSheet("QPushButton { background-color: #80FF0000;}");
}

void ToolEditForm::flicker(QDoubleSpinBox* dsbx)
{
    if (!dsbx->value()) {
        for (int i = 0, t = 0; i < 3; ++i) {
            QTimer::singleShot(++t * 150, Qt::CoarseTimer, [dsbx] { dsbx->setStyleSheet("QDoubleSpinBox { background-color: #FF0000;}"); });
            QTimer::singleShot(++t * 150, Qt::CoarseTimer, [dsbx] { dsbx->setStyleSheet(""); });
        }
    }
}

void ToolEditForm::setVisibleWidgets(bool visible)
{
    ui->cbxFeedSpeeds->setVisible(visible);
    ui->cbxToolType->setVisible(visible);
    ui->cbxUnits->setVisible(visible);
    ui->groupBox_2->setVisible(visible);
    ui->groupBox_3->setVisible(visible);
    ui->groupBox_4->setVisible(visible);
    ui->lblPixmap->setVisible(visible);
    ui->lblToolType->setVisible(visible);
    ui->lblUnits->setVisible(visible);
}

void ToolEditForm::on_cbxFeedSpeeds_currentIndexChanged(int index)
{
    double feed = m_feed;
    switch (index) {
    case 0: //mm/sec
        m_feed = 1.0 / 60.0;
        break;
    case 1: //mm/min!!!
        m_feed = 1.0;
        break;
    case 2: //cm/min
        m_feed = 1.0 / 10.0;
        break;
    case 3: //m/min
        m_feed = 1.0 / 1000.0;
        break;
    default:
        break;
    }
    QString str(ui->pbApply->styleSheet());
    ui->dsbxFeedRate->setValue((ui->dsbxFeedRate->value() / feed) * m_feed);
    ui->dsbxPlungeRate->setValue((ui->dsbxPlungeRate->value() / feed) * m_feed);
    ui->dsbxFeedRate->setSuffix(" " + ui->cbxFeedSpeeds->currentText());
    ui->dsbxPlungeRate->setSuffix(" " + ui->cbxFeedSpeeds->currentText());
    ui->pbApply->setStyleSheet(str);
}

void ToolEditForm::on_cbxToolType_currentIndexChanged(int index)
{
    ui->dsbxAngle->setEnabled(true);
    ui->dsbxFeedRate->setEnabled(true);
    ui->dsbxStepover->setEnabled(true);
    ui->dsbxStepoverPercent->setEnabled(true);
    switch (index) {
    case Tool::Drill:
        ui->dsbxFeedRate->setEnabled(false);
        ui->dsbxStepover->setEnabled(false);
        ui->dsbxStepoverPercent->setEnabled(false);
        ui->lblPixmap->setPixmap(QPixmap(QString::fromUtf8(":/1.png")));
        ui->label_3->setText("Pass");
        break;
    case Tool::EndMill:
        ui->dsbxAngle->setEnabled(false);
        ui->lblPixmap->setPixmap(QPixmap(QString::fromUtf8(":/2.png")));
        ui->label_3->setText("Depth");
        break;
    case Tool::Engraving:
        ui->lblPixmap->setPixmap(QPixmap(QString::fromUtf8(":/3.png")));
        ui->label_3->setText("Depth");
        break;
    }

    ui->dsbxAngle->setMaximum(ui->dsbxAngle->isEnabled() ? 180.0 : 0.0);
    ui->dsbxFeedRate->setMaximum(ui->dsbxFeedRate->isEnabled() ? 100000.0 : 0.0);
    ui->dsbxStepover->setMaximum(ui->dsbxStepover->isEnabled() ? ui->dsbxDiameter->value() : 0.0);
    ui->dsbxStepoverPercent->setMaximum(ui->dsbxStepoverPercent->isEnabled() ? 100.0 : 0.0);

    m_tool.type = static_cast<Tool::Type>(index);
    setRed();
    updateName();
}

void ToolEditForm::on_leName_textChanged(const QString& arg1)
{
    m_tool.name = arg1;
    setRed();
}

void ToolEditForm::on_teNote_textChanged()
{
    m_tool.note = ui->teNote->toPlainText();
    setRed();
}

void ToolEditForm::calculate(int index, double value)
{
    switch (index) {
    case Tool::Angle:
        m_tool.angle = value;
        break;
    case Tool::Diameter:
        m_tool.diameter = value;
        ui->dsbxOneTurnCut->setMaximum(value);
        ui->dsbxStepover->setMaximum(value);
        //        ui->dsbxOneTurnCutPercent->setValue((ui->dsbxOneTurnCut->value() / value) * 100.0);
        //        ui->dsbxStepoverPercent->setValue((ui->dsbxStepover->value() / value) * 100.0);
        ui->dsbxOneTurnCutPercent->valueChanged(ui->dsbxOneTurnCutPercent->value());
        ui->dsbxStepoverPercent->valueChanged(ui->dsbxStepoverPercent->value());
        break;
    case Tool::FeedRate:
        m_tool.feedRate = value / m_feed;
        break;
    case Tool::OneTurnCut:
        m_tool.oneTurnCut = value;
        ui->dsbxOneTurnCutPercent->setValue(value / (m_tool.diameter * 0.01));
        if (ui->chbxFeedRate->isChecked())
            ui->dsbxFeedRate->setValue(m_tool.oneTurnCut * m_tool.spindleSpeed * m_feed);
        if (ui->chbxPlungeRate->isChecked())
            ui->dsbxPlungeRate->setValue(m_tool.oneTurnCut * m_tool.spindleSpeed * m_feed);
        break;
    case Tool::PassDepth:
        m_tool.passDepth = value;
        break;
    case Tool::PlungeRate:
        m_tool.plungeRate = value / m_feed;
        break;
    case Tool::SpindleSpeed:
        m_tool.spindleSpeed = value; //rpm
        if (ui->chbxFeedRate->isChecked())
            ui->dsbxFeedRate->setValue(m_tool.oneTurnCut * m_tool.spindleSpeed * m_feed);
        if (ui->chbxPlungeRate->isChecked())
            ui->dsbxPlungeRate->setValue(m_tool.oneTurnCut * m_tool.spindleSpeed * m_feed);
        break;
    case Tool::Stepover:
        m_tool.stepover = value;
        ui->dsbxStepoverPercent->setValue(value / (m_tool.diameter * 0.01));
        break;
    case Tool::OneTurnCutPercent:
        ui->dsbxOneTurnCut->setValue(value * (m_tool.diameter * 0.01));
        break;
    case Tool::StepoverPercent:
        ui->dsbxStepover->setValue(value * (m_tool.diameter * 0.01));
        break;
    default:
        break;
    }
}

void ToolEditForm::valueChanged(double value)
{
    const QVector<void*> set{
        ui->dsbxAngle,
        ui->dsbxDiameter,
        ui->dsbxFeedRate,
        ui->dsbxOneTurnCut,
        ui->dsbxPassDepth,
        ui->dsbxPlungeRate,
        ui->dsbxSpindleSpeed,
        ui->dsbxStepover,
        ui->dsbxOneTurnCutPercent,
        ui->dsbxStepoverPercent,
    };
    calculate(set.indexOf(sender()), value);
    updateName();
    setRed();
}

void ToolEditForm::on_pbApply_clicked()
{
    if (m_item && m_tool.isValid()) {
        m_item->tool() = m_tool;
        m_item->setName(m_tool.name);
        m_item->setNote(m_tool.note);
        emit itemChanged(m_item);
        ui->pbApply->setStyleSheet("");
    } else {
        switch (m_tool.type) {
        case Tool::Drill:
            flicker(ui->dsbxDiameter);
            flicker(ui->dsbxOneTurnCut);
            flicker(ui->dsbxPassDepth);
            flicker(ui->dsbxPlungeRate);
            flicker(ui->dsbxSpindleSpeed);
            flicker(ui->dsbxOneTurnCutPercent);
            break;
        case Tool::EndMill:
            flicker(ui->dsbxDiameter);
            flicker(ui->dsbxFeedRate);
            flicker(ui->dsbxOneTurnCut);
            flicker(ui->dsbxPassDepth);
            flicker(ui->dsbxPlungeRate);
            flicker(ui->dsbxSpindleSpeed);
            flicker(ui->dsbxStepover);
            flicker(ui->dsbxOneTurnCutPercent);
            flicker(ui->dsbxStepoverPercent);
            break;
        case Tool::Engraving:
            flicker(ui->dsbxDiameter);
            flicker(ui->dsbxFeedRate);
            flicker(ui->dsbxOneTurnCut);
            flicker(ui->dsbxPassDepth);
            flicker(ui->dsbxPlungeRate);
            flicker(ui->dsbxSpindleSpeed);
            flicker(ui->dsbxStepover);
            flicker(ui->dsbxOneTurnCutPercent);
            flicker(ui->dsbxStepoverPercent);
        case Tool::Group:
        default:
            m_item->tool() = m_tool;
            m_item->setName(m_tool.name);
            m_item->setNote(m_tool.note);
            emit itemChanged(m_item);
            ui->pbApply->setStyleSheet("");
            break;
        }
    }
}

void ToolEditForm::setTool(const Tool& tool)
{
    ui->dsbxAngle->setValue(tool.angle);
    ui->dsbxDiameter->setValue(tool.diameter);
    ui->dsbxOneTurnCut->setValue(tool.oneTurnCut);
    ui->dsbxPassDepth->setValue(tool.passDepth);
    ui->dsbxSpindleSpeed->setValue(tool.spindleSpeed);
    ui->dsbxStepover->setValue(tool.stepover);

    ui->cbxToolType->setCurrentIndex(tool.type);
    ui->leName->setText(tool.name);
    ui->teNote->setText(tool.note);

    ui->dsbxFeedRate->setValue(tool.feedRate * m_feed);
    ui->dsbxPlungeRate->setValue(tool.plungeRate * m_feed);

    m_tool = tool;
    setVisibleWidgets(true);
}

void ToolEditForm::updateName()
{
    switch (ui->cbxToolType->currentIndex()) {
    case Tool::EndMill:
        ui->leName->setText(QString(tr("End Mill (Ø%1 mm)")).arg(ui->dsbxDiameter->value()));
        break;
    case Tool::Engraving:
        ui->leName->setText(QString(tr("Engrave (%2\302\260 %1 mm tip)")).arg(ui->dsbxDiameter->value()).arg(ui->dsbxAngle->value()));
        break;
    case Tool::Drill:
        ui->leName->setText(QString(tr("Drill (Ø%1 mm)")).arg(ui->dsbxDiameter->value()));
        break;
    }
}
