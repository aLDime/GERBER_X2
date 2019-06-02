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

    connect(ui->dsbxAngle, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolEditForm::valueChanged);
    connect(ui->dsbxDiameter, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolEditForm::valueChanged);
    connect(ui->dsbxFeedRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolEditForm::valueChanged);
    connect(ui->dsbxOneTurnCut, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolEditForm::valueChanged);
    connect(ui->dsbxOneTurnCutPercent, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolEditForm::valueChanged);
    connect(ui->dsbxPassDepth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolEditForm::valueChanged);
    connect(ui->dsbxPlungeRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolEditForm::valueChanged);
    connect(ui->dsbxSpindleSpeed, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolEditForm::valueChanged);
    connect(ui->dsbxStepover, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolEditForm::valueChanged);
    connect(ui->dsbxStepoverPercent, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolEditForm::valueChanged);

    connect(ui->cbxFeedSpeeds, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
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
        bool fl1 = parentWidget()->isWindowModified();
        bool fl2 = ui->pbApply->isEnabled();
        ui->dsbxFeedRate->setValue((ui->dsbxFeedRate->value() / feed) * m_feed);
        ui->dsbxPlungeRate->setValue((ui->dsbxPlungeRate->value() / feed) * m_feed);
        ui->dsbxFeedRate->setSuffix(" " + ui->cbxFeedSpeeds->currentText());
        ui->dsbxPlungeRate->setSuffix(" " + ui->cbxFeedSpeeds->currentText());
        parentWidget()->setWindowModified(fl1);
        ui->pbApply->setEnabled(fl2);
    });

    connect(ui->cbxToolType, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        ui->dsbxAngle->setEnabled(true);
        ui->dsbxFeedRate->setEnabled(true);
        ui->dsbxStepover->setEnabled(true);
        ui->dsbxStepoverPercent->setEnabled(true);
        switch (index) {
        case Tool::Drill:
            ui->dsbxFeedRate->setEnabled(false);
            ui->dsbxStepover->setEnabled(false);
            ui->dsbxStepoverPercent->setEnabled(false);
            if (ui->lblPixmap)
                ui->lblPixmap->setPixmap(QPixmap(QString::fromUtf8(":/tool/drill.png")));
            ui->label_3->setText(tr("Pass"));
            break;
        case Tool::EndMill:
            ui->dsbxAngle->setEnabled(false);
            if (ui->lblPixmap)
                ui->lblPixmap->setPixmap(QPixmap(QString::fromUtf8(":/tool/endmill.png")));
            ui->label_3->setText(tr("Depth"));
            if (ui->dsbxFeedRate->value() == 0.0) {
                //                ui->dsbxStepover->setValue(ui->dsbxDiameter->value() * 0.5);
                //                ui->dsbxStepoverPercent->setValue(ui->dsbxStepover->value() / (m_tool.diameter * 0.01));
                ui->dsbxFeedRate->setValue(m_tool.oneTurnCut * m_tool.spindleSpeed * m_feed);
            }
            break;
        case Tool::Engraving:
            if (ui->lblPixmap)
                ui->lblPixmap->setPixmap(QPixmap(QString::fromUtf8(":/tool/engraving.png")));
            ui->label_3->setText(tr("Depth"));
            if (ui->dsbxFeedRate->value() == 0.0) {
                //                ui->dsbxStepover->setValue(ui->dsbxDiameter->value() * 0.5);
                //                ui->dsbxStepoverPercent->setValue(ui->dsbxStepover->value() / (m_tool.diameter * 0.01));
                ui->dsbxFeedRate->setValue(m_tool.oneTurnCut * m_tool.spindleSpeed * m_feed);
            }
            break;
        }

        //        ui->dsbxAngle->setMaximum(ui->dsbxAngle->isEnabled() ? 180.0 : 0.0);
        ui->dsbxFeedRate->setMaximum(ui->dsbxFeedRate->isEnabled() ? 100000.0 : 0.0);
        //        ui->dsbxStepover->setMaximum(ui->dsbxStepover->isEnabled() ? ui->dsbxDiameter->value() : 0.0);
        //        ui->dsbxStepoverPercent->setMaximum(ui->dsbxStepoverPercent->isEnabled() ? 100.0 : 0.0);

        m_tool.type = static_cast<Tool::Type>(index);
        setRed();
        updateName();
    });

    connect(ui->leName, &QLineEdit::textChanged, [=](const QString& arg1) { m_tool.name = arg1; setRed(); });
    connect(ui->leName, &QLineEdit::textEdited, [=](const QString& arg1) { m_tool.name = arg1; setRed();  ui->chbxAutoName->setChecked(false); });
    connect(ui->teNote, &QTextEdit::textChanged, [=] { m_tool.note = ui->teNote->toPlainText(); setRed(); });

    set = {
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

    ui->cbxUnits->setVisible(false);

    QSettings settings;
    ui->cbxFeedSpeeds->setCurrentIndex(settings.value("cbxFeedSpeeds").toInt());
    //ui->cbxToolType->setCurrentIndex(settings.value("cbxToolType").toInt());
    //ui->cbxUnits->setCurrentIndex(settings.value("cbxUnits").toInt());

    setVisibleWidgets(false);
    setRedReset();
}

ToolEditForm::~ToolEditForm()
{
    QSettings settings;
    settings.setValue("cbxFeedSpeeds", ui->cbxFeedSpeeds->currentIndex());
    //settings.setValue("cbxToolType", ui->cbxToolType->currentIndex());
    //settings.setValue("cbxUnits", ui->cbxUnits->currentIndex());

    delete ui;
    ui = nullptr;
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
    setRedReset();
    parentWidget()->setWindowModified(false);
}

void ToolEditForm::setRed()
{
    //if (m_item)
    ui->pbApply->setEnabled(true);
    parentWidget()->setWindowModified(true);
}

void ToolEditForm::setRedReset()
{
    ui->pbApply->setEnabled(false);
    parentWidget()->setWindowModified(false);
}

void ToolEditForm::flicker(QDoubleSpinBox* dsbx)
{
    if (!dsbx->value()) {
        for (int i = 0, t = 0; i < 3; ++i) {
            QTimer::singleShot(++t * 150, Qt::CoarseTimer, [dsbx, this] { if(!ui) return; dsbx->setStyleSheet("QDoubleSpinBox { background-color: #FF0000;}"); });
            QTimer::singleShot(++t * 150, Qt::CoarseTimer, [dsbx, this] { if(!ui) return; dsbx->setStyleSheet(""); });
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
    if (ui->lblPixmap)
        ui->lblPixmap->setVisible(visible);
    ui->lblToolType->setVisible(visible);
    ui->lblUnits->setVisible(visible);
    ui->chbxAutoName->setVisible(visible);
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
        // ui->dsbxOneTurnCutPercent->setValue((ui->dsbxOneTurnCut->value() / value) * 100.0);
        // ui->dsbxStepoverPercent->setValue((ui->dsbxStepover->value() / value) * 100.0);
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
        setRedReset();
    } else {
        m_tool.autoName = ui->chbxAutoName->isChecked();
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
            if (!m_item) {
                setRedReset();
                break;
            }
            m_item->tool() = m_tool;
            // m_item->setName(m_tool.name);
            // m_item->setNote(m_tool.note);
            emit itemChanged(m_item);
            setRedReset();
            break;
        }
    }
}

void ToolEditForm::setDialog()
{
    m_dialog = false;
    ui->cbxToolType->setEnabled(m_dialog);
    delete ui->lblPixmap;
    ui->lblPixmap = nullptr;
}

void ToolEditForm::setTool(const Tool& tool)
{
    m_tool = tool;
    ui->dsbxAngle->setValue(tool.angle);
    ui->dsbxDiameter->setValue(tool.diameter);
    ui->dsbxPassDepth->setValue(tool.passDepth);
    ui->dsbxSpindleSpeed->setValue(tool.spindleSpeed);
    ui->dsbxStepover->setValue(tool.stepover);
    ui->dsbxOneTurnCut->setValue(tool.oneTurnCut);

    ui->cbxToolType->setCurrentIndex(tool.type);
    ui->leName->setText(tool.name);
    ui->teNote->setText(tool.note);

    ui->dsbxFeedRate->setValue(tool.feedRate * m_feed);
    ui->dsbxPlungeRate->setValue(tool.plungeRate * m_feed);

    ui->chbxAutoName->setChecked(tool.autoName);
    setVisibleWidgets(true);
}

void ToolEditForm::updateName()
{
    if (ui->chbxAutoName->isChecked()) {
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
}
