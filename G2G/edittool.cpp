#include "edittool.h"

#include <QGroupBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QMessageBox>

EditTool::EditTool(QWidget* parent)
    : QWidget(parent)
    , dsbList(10)
{
    setupUi(this);
    //    setStyleSheet("QGroupBox {"
    //                  "    background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF);"
    //                  "    border: 2px solid gray;"
    //                  "    border-radius: 5px;"
    //                  "    margin-top: 1ex; /* leave space at the top for the title */"
    //                  "}");

    cbxToolType->addItems(QStringLiteral("End Mill|Engraving|Drill").split("|"));
    cbxFeedSpeeds->addItems(QStringLiteral("mm/sec|mm/min|m/min").split("|"));
    on_cbxToolType_activated(0);
}

EditTool::~EditTool()
{
    QApplication::exit(0);
}

void EditTool::on_cbxToolType_activated(int index)
{
    for (QDoubleSpinBox* dsb: dsbList) {
        dsb->setEnabled(true);
    }

    if (index != lastIndex && QMessageBox::question(this, "!!!", "Yes?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No) {
        cbxToolType->setCurrentIndex(lastIndex);
        return;
    }

    lastIndex = cbxToolType->currentIndex();

    switch (index) {
    case EndMill:
        dsbList[ClearencePassStepover]->setEnabled(false);
        dsbList[ClearencePassStepover]->setValue(0);
        dsbList[ClearencePassStepoverPercent]->setEnabled(false);
        dsbList[ClearencePassStepoverPercent]->setValue(0);
        dsbList[FlatDiameter]->setEnabled(false);
        dsbList[FlatDiameter]->setValue(0);
        dsbList[SideAngle]->setEnabled(false);
        dsbList[SideAngle]->setValue(0);
        lblPixmap->setPixmap(QPixmap(QString::fromUtf8(":/tool/endmill.png")));

        break;
    case Engraving:
        lblPixmap->setPixmap(QPixmap(QString::fromUtf8(":/tool/engraving.png")));

        break;
    case Drill:
        dsbList[ClearencePassStepover]->setEnabled(false);
        dsbList[ClearencePassStepover]->setValue(0);
        dsbList[ClearencePassStepoverPercent]->setEnabled(false);
        dsbList[ClearencePassStepoverPercent]->setValue(0);
        dsbList[FeedRate]->setEnabled(false);
        dsbList[FeedRate]->setValue(0);
        dsbList[FlatDiameter]->setEnabled(false);
        dsbList[FlatDiameter]->setValue(0);
        dsbList[Stepover]->setEnabled(false);
        dsbList[Stepover]->setValue(0);
        dsbList[StepoverPercent]->setEnabled(false);
        dsbList[StepoverPercent]->setValue(0);
        lblPixmap->setPixmap(QPixmap(QString::fromUtf8(":/tool/drill.png")));

        break;
    default:
        break;
    }
    updateName();
}

TOOL EditTool::getTool()
{
    for (int i = 0; i < dsbList.size(); ++i) {
        tool.Params[i] = dsbList[i]->value();
    }
    //    tool.ClearencePassStepover = dsbList[ClearencePassStepover]->value();
    //    tool.ClearencePassStepoverPercent = dsbList[ClearencePassStepoverPercent]->value();
    //    tool.Diameter = dsbList[Diameter]->value();
    //    tool.FeedRate = dsbList[FeedRate]->value();
    //    tool.FlatDiameter = dsbList[FlatDiameter]->value();
    //    tool.PassDepth = dsbList[PassDepth]->value();
    //    tool.PlungeRate = dsbList[PlungeRate]->value();
    //    tool.SideAngle = dsbList[SideAngle]->value();
    //    tool.Stepover = dsbList[Stepover]->value();
    //    tool.StepoverPercent = dsbList[StepoverPercent]->value();
    tool.FeedSpeeds = cbxFeedSpeeds->currentIndex();
    tool.SpindleSpeed = sbSpindleSpeed->value();
    tool.ToolType = cbxToolType->currentIndex();
    tool.Name = leName->text();
    tool.Note = pteNote->document()->toRawText();
    return tool;
}

void EditTool::setTool(const TOOL& value)
{
    isNew = false;
    tool = value;
    for (int i = 0; i < dsbList.size(); ++i) {
        dsbList[i]->setValue(tool.Params[i]);
    }
    //    dsbList[ClearencePassStepover]->setValue(tool.ClearencePassStepover);
    //    dsbList[ClearencePassStepoverPercent]->setValue(tool.ClearencePassStepoverPercent);
    //    dsbList[Diameter]->setValue(tool.Diameter);
    //    dsbList[FeedRate]->setValue(tool.FeedRate);
    //    dsbList[FlatDiameter]->setValue(tool.FlatDiameter);
    //    dsbList[PassDepth]->setValue(tool.PassDepth);
    //    dsbList[PlungeRate]->setValue(tool.PlungeRate);
    //    dsbList[SideAngle]->setValue(tool.SideAngle);
    //    dsbList[Stepover]->setValue(tool.Stepover);
    //    dsbList[StepoverPercent]->setValue(tool.StepoverPercent);
    cbxFeedSpeeds->setCurrentIndex(tool.FeedSpeeds);
    sbSpindleSpeed->setValue(tool.SpindleSpeed);
    cbxToolType->setCurrentIndex(tool.ToolType);
    leName->setText(tool.Name);
    pteNote->document()->setPlainText(tool.Note);
}

void EditTool::setupUi(QWidget* EditTool)
{
    if (EditTool->objectName().isEmpty())
        EditTool->setObjectName(QStringLiteral("EditTool"));

    //    EditTool->resize(100, 100);
    QFont font1;
    font1.setBold(false);
    font1.setWeight(50);

    groupBox = new QGroupBox(EditTool);
    groupBox->setObjectName(QStringLiteral("groupBox"));
    QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
    groupBox->setSizePolicy(sizePolicy);

    label_5 = new QLabel(groupBox);
    label_5->setObjectName(QStringLiteral("label_5"));
    label_5->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    leName = new QLineEdit(groupBox);
    leName->setObjectName(QStringLiteral("leName"));
    //    QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Fixed);
    //    sizePolicy1.setHorizontalStretch(0);
    //    sizePolicy1.setVerticalStretch(0);
    //    sizePolicy1.setHeightForWidth(leName->sizePolicy().hasHeightForWidth());
    //    leName->setSizePolicy(sizePolicy1);

    label_6 = new QLabel(groupBox);
    label_6->setObjectName(QStringLiteral("label_6"));
    label_6->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    cbxToolType = new QComboBox(groupBox);
    cbxToolType->setObjectName(QStringLiteral("cbxToolType"));
    //    sizePolicy1.setHeightForWidth(cbxToolType->sizePolicy().hasHeightForWidth());
    //    cbxToolType->setSizePolicy(sizePolicy1);

    label_7 = new QLabel(groupBox);
    label_7->setObjectName(QStringLiteral("label_7"));
    label_7->setAlignment(Qt::AlignRight | Qt::AlignTop | Qt::AlignTrailing);

    pteNote = new QPlainTextEdit(groupBox);
    pteNote->setObjectName(QStringLiteral("pteNote"));
    //    QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Expanding);
    //    sizePolicy2.setHorizontalStretch(0);
    //    sizePolicy2.setVerticalStretch(0);
    //    sizePolicy2.setHeightForWidth(pteNote->sizePolicy().hasHeightForWidth());
    //    pteNote->setSizePolicy(sizePolicy2);
    pteNote->setMaximumSize(QSize(16777215, 100));

    label_14 = new QLabel(groupBox);
    label_14->setObjectName(QStringLiteral("label_14"));
    label_14->setEnabled(true);
    label_14->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    cbxUnits = new QComboBox(groupBox);
    cbxUnits->setObjectName(QStringLiteral("cbxUnits"));
    cbxUnits->setEnabled(false);
    //    sizePolicy1.setHeightForWidth(cbxUnits->sizePolicy().hasHeightForWidth());
    //    cbxUnits->setSizePolicy(sizePolicy1);

    grBox_2 = new QGroupBox(groupBox);
    grBox_2->setObjectName(QStringLiteral("groupBox_2"));
    //    QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Preferred);
    //    sizePolicy3.setHorizontalStretch(0);
    //    sizePolicy3.setVerticalStretch(0);
    //    sizePolicy3.setHeightForWidth(groupBox_2->sizePolicy().hasHeightForWidth());
    //    groupBox_2->setSizePolicy(sizePolicy3);
    QFont font;
    //    font.setBold(true);
    //    font.setWeight(75);
    grBox_2->setFont(font);
    grBox_2->setFlat(false);
    label_8 = new QLabel(grBox_2);
    label_8->setObjectName(QStringLiteral("label_8"));

    label_8->setFont(font1);
    label_8->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    label_9 = new QLabel(grBox_2);
    label_9->setObjectName(QStringLiteral("label_9"));
    label_9->setFont(font1);
    label_9->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    label_10 = new QLabel(grBox_2);
    label_10->setObjectName(QStringLiteral("label_10"));
    label_10->setFont(font1);
    label_10->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    dsbList[Diameter] = new QDoubleSpinBox(grBox_2);
    dsbList[Diameter]->setObjectName(QStringLiteral("Diameter"));
    dsbList[Diameter]->setFont(font1);
    dsbList[Diameter]->setMaximum(100);
    dsbList[SideAngle] = new QDoubleSpinBox(grBox_2);
    dsbList[SideAngle]->setObjectName(QStringLiteral("SideAngle"));
    dsbList[SideAngle]->setFont(font1);
    dsbList[SideAngle]->setMaximum(180);
    dsbList[FlatDiameter] = new QDoubleSpinBox(grBox_2);
    dsbList[FlatDiameter]->setObjectName(QStringLiteral("FlatDiameter"));
    dsbList[FlatDiameter]->setFont(font1);
    dsbList[FlatDiameter]->setMaximum(100);

    QGridLayout* gridLayout_3 = new QGridLayout(grBox_2);
    gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
    gridLayout_3->setContentsMargins(6, 6, 6, 6);
    gridLayout_3->addWidget(label_8, 0, 0, 1, 1);
    gridLayout_3->addWidget(dsbList[Diameter], 0, 1, 1, 1);
    gridLayout_3->addWidget(label_9, 1, 0, 1, 1);
    gridLayout_3->addWidget(dsbList[SideAngle], 1, 1, 1, 1);
    gridLayout_3->addWidget(label_10, 2, 0, 1, 1);
    gridLayout_3->addWidget(dsbList[FlatDiameter], 2, 1, 1, 1);

    grBox_3 = new QGroupBox(groupBox);
    grBox_3->setObjectName(QStringLiteral("groupBox_3"));
    //    sizePolicy3.setHeightForWidth(groupBox_3->sizePolicy().hasHeightForWidth());
    //    groupBox_3->setSizePolicy(sizePolicy3);
    grBox_3->setFont(font);
    label_11 = new QLabel(grBox_3);
    label_11->setObjectName(QStringLiteral("label_11"));
    label_11->setFont(font1);
    label_11->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    label_12 = new QLabel(grBox_3);
    label_12->setObjectName(QStringLiteral("label_12"));
    label_12->setFont(font1);
    label_12->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    label_13 = new QLabel(grBox_3);
    label_13->setObjectName(QStringLiteral("label_13"));
    label_13->setFont(font1);
    label_13->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    dsbList[PassDepth] = new QDoubleSpinBox(grBox_3);
    dsbList[PassDepth]->setObjectName(QStringLiteral("PassDepth"));
    dsbList[PassDepth]->setFont(font1);
    dsbList[PassDepth]->setMaximum(100);
    dsbList[Stepover] = new QDoubleSpinBox(grBox_3);
    dsbList[Stepover]->setObjectName(QStringLiteral("Stepover"));
    dsbList[Stepover]->setFont(font1);
    dsbList[Stepover]->setMaximum(100);
    dsbList[StepoverPercent] = new QDoubleSpinBox(grBox_3);
    dsbList[StepoverPercent]->setObjectName(QStringLiteral("StepoverPercent"));
    dsbList[StepoverPercent]->setFont(font1);
    dsbList[StepoverPercent]->setMaximum(100);
    dsbList[ClearencePassStepover] = new QDoubleSpinBox(grBox_3);
    dsbList[ClearencePassStepover]->setObjectName(QStringLiteral("ClearencePassStepover"));
    dsbList[ClearencePassStepover]->setFont(font1);
    dsbList[ClearencePassStepover]->setMaximum(100);
    dsbList[ClearencePassStepoverPercent] = new QDoubleSpinBox(grBox_3);
    dsbList[ClearencePassStepoverPercent]->setObjectName(QStringLiteral("ClearencePassStepoverPercent"));
    dsbList[ClearencePassStepoverPercent]->setFont(font1);
    dsbList[ClearencePassStepoverPercent]->setMaximum(100);

    QGridLayout* gridLayout = new QGridLayout(grBox_3);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));
    gridLayout->setContentsMargins(6, 6, 6, 6);
    gridLayout->addWidget(label_11, 0, 0, 1, 1);
    gridLayout->addWidget(dsbList[PassDepth], 0, 1, 1, 1);
    gridLayout->addWidget(label_12, 1, 0, 1, 1);
    gridLayout->addWidget(dsbList[Stepover], 1, 1, 1, 1);
    gridLayout->addWidget(dsbList[StepoverPercent], 1, 2, 1, 1);
    gridLayout->addWidget(label_13, 2, 0, 1, 1);
    gridLayout->addWidget(dsbList[ClearencePassStepover], 2, 1, 1, 1);
    gridLayout->addWidget(dsbList[ClearencePassStepoverPercent], 2, 2, 1, 1);

    grBox_4 = new QGroupBox(groupBox);
    grBox_4->setObjectName(QStringLiteral("groupBox_4"));
    //    sizePolicy3.setHeightForWidth(groupBox_4->sizePolicy().hasHeightForWidth());
    //    groupBox_4->setSizePolicy(sizePolicy3);
    grBox_4->setFont(font);
    label_2 = new QLabel(grBox_4);
    label_2->setObjectName(QStringLiteral("label_2"));
    label_2->setFont(font1);
    label_2->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    label_3 = new QLabel(grBox_4);
    label_3->setObjectName(QStringLiteral("label_3"));
    label_3->setFont(font1);
    label_3->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    label_4 = new QLabel(grBox_4);
    label_4->setObjectName(QStringLiteral("label_4"));
    label_4->setFont(font1);
    label_4->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    sbSpindleSpeed = new QSpinBox(grBox_4);
    sbSpindleSpeed->setObjectName(QStringLiteral("sbSpindleSpeed"));
    sbSpindleSpeed->setFont(font1);
    sbSpindleSpeed->setMaximum(50000);

    cbxFeedSpeeds = new QComboBox(grBox_4);
    cbxFeedSpeeds->setObjectName(QStringLiteral("cbxFeedpSpeeds"));

    dsbList[PlungeRate] = new QDoubleSpinBox(grBox_4);
    dsbList[PlungeRate]->setObjectName(QStringLiteral("PlungeRate"));
    dsbList[PlungeRate]->setFont(font1);
    dsbList[PlungeRate]->setMaximum(100);
    dsbList[FeedRate] = new QDoubleSpinBox(grBox_4);
    dsbList[FeedRate]->setObjectName(QStringLiteral("FeedRate"));
    dsbList[FeedRate]->setFont(font1);
    dsbList[FeedRate]->setMaximum(100);

    QGridLayout* gridLayout_2 = new QGridLayout(grBox_4);
    gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
    gridLayout_2->setContentsMargins(6, 6, 6, 6);
    gridLayout_2->addWidget(label_2, 1, 0, 1, 1);
    gridLayout_2->addWidget(label_3, 0, 0, 1, 1);
    gridLayout_2->addWidget(label_4, 2, 0, 1, 1);
    gridLayout_2->addWidget(sbSpindleSpeed, 0, 1, 1, 1);
    gridLayout_2->addWidget(dsbList[PlungeRate], 2, 1, 1, 1);
    gridLayout_2->addWidget(dsbList[FeedRate], 1, 1, 1, 1);
    gridLayout_2->addWidget(cbxFeedSpeeds, 1, 2, 2, 1);

    bpApply = new QPushButton(groupBox);
    bpApply->setObjectName(QStringLiteral("bpApply"));
    QSizePolicy sizePolicy4(QSizePolicy::Minimum, QSizePolicy::Minimum);
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(bpApply->sizePolicy().hasHeightForWidth());
    bpApply->setSizePolicy(sizePolicy4);

    lblPixmap = new QLabel(groupBox);
    lblPixmap->setObjectName(QStringLiteral("label"));
    lblPixmap->setPixmap(QPixmap(QString::fromUtf8(":/tool/endmill.png")));
    lblPixmap->setAlignment(Qt::AlignCenter);

    QGridLayout* gridLayout_4 = new QGridLayout(groupBox);
    gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
    gridLayout_4->addWidget(label_5, 0, 0, 1, 1);
    gridLayout_4->addWidget(leName, 0, 1, 1, 1);
    gridLayout_4->addWidget(label_6, 1, 0, 1, 1);
    gridLayout_4->addWidget(cbxToolType, 1, 1, 1, 1);
    gridLayout_4->addWidget(label_7, 2, 0, 1, 1);
    gridLayout_4->addWidget(pteNote, 2, 1, 1, 1);
    gridLayout_4->addWidget(label_14, 3, 0, 1, 1);
    gridLayout_4->addWidget(cbxUnits, 3, 1, 1, 1);
    gridLayout_4->addWidget(grBox_2, 4, 0, 1, 2);
    gridLayout_4->addWidget(grBox_3, 5, 0, 1, 2);
    gridLayout_4->addWidget(grBox_4, 6, 0, 1, 2);
    gridLayout_4->addWidget(bpApply, 6, 2, 1, 1);
    gridLayout_4->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding), 7, 1, 1, 1);
    gridLayout_4->addWidget(lblPixmap, 0, 2, 6, 1);

    QVBoxLayout* verticalLayout = new QVBoxLayout(EditTool);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    verticalLayout->addWidget(groupBox);
    verticalLayout->setMargin(0);
    ///////////////////////////////////////////////////////////////////////

    dsbList[Diameter]->setSingleStep(0.1);
    dsbList[FlatDiameter]->setSingleStep(0.1);

    connect(dsbList[Diameter], static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double) { updateName(); });
    connect(dsbList[SideAngle], static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double) { updateName(); });
    connect(dsbList[FlatDiameter], static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double) { updateName(); });

    ///////////////////////////////////////////////////////////////////////
    retranslateUi(EditTool);
    QMetaObject::connectSlotsByName(EditTool);
}

void EditTool::retranslateUi(QWidget* EditTool)
{
    EditTool->setWindowTitle(tr("Dialog"));
    bpApply->setText(tr("Apply"));
    cbxFeedSpeeds->clear();
    cbxFeedSpeeds->insertItems(0, QStringList() << tr("mm/sec") << tr("mm/min") << tr("m/min"));
    cbxUnits->clear();
    cbxUnits->insertItems(0, QStringList() << tr("mm") << tr("inches"));

    dsbList[ClearencePassStepover]->setSuffix(tr(" mm"));
    dsbList[ClearencePassStepoverPercent]->setSuffix(tr(" %"));
    dsbList[Diameter]->setSuffix(tr(" mm"));
    dsbList[FlatDiameter]->setSuffix(tr(" mm"));
    dsbList[PassDepth]->setSuffix(tr(" mm"));
    dsbList[SideAngle]->setSuffix(tr(" \302\260"));
    dsbList[Stepover]->setSuffix(tr(" mm"));
    dsbList[StepoverPercent]->setSuffix(tr(" %"));

    grBox_2->setTitle(tr("Geometry"));
    grBox_3->setTitle(tr("Cutting Parameters"));
    grBox_4->setTitle(tr("Feedp Speeds"));
    groupBox->setTitle(tr("Tool Info"));
    lblPixmap->setText(QString());
    label_10->setText(tr("Flat Diameter"));
    label_11->setText(tr("Pass Depth"));
    label_12->setText(tr("Stepover"));
    label_13->setText(tr("Clearence Pass\nStepover"));
    label_14->setText(tr("Units"));
    label_2->setText(tr("Feed Rate"));
    label_3->setText(tr("Spindle Speed"));
    label_4->setText(tr("Plunge Rate"));
    label_5->setText(tr("Name"));
    label_6->setText(tr("Tool Type"));
    label_7->setText(tr("Note"));
    label_8->setText(tr("Diameter"));
    label_9->setText(tr("Side/Included Angle"));
    sbSpindleSpeed->setSuffix(tr(" r.p.m."));

    groupBox->setEnabled(false);
}

void EditTool::updateName()
{
    if (!isNew)
        return;
    switch (cbxToolType->currentIndex()) {
    case EndMill:
        leName->setText(QString(tr("End Mill (%1 mm)")).arg(dsbList[Diameter]->value()));
        break;
    case Engraving:
        leName->setText(QString(tr("Engrave (%2\302\260 %1 mm Tip Dia)")).arg(dsbList[FlatDiameter]->value()).arg(dsbList[SideAngle]->value()));
        break;
    case Drill:
        leName->setText(QString(tr("Drill (%1 mm)")).arg(dsbList[Diameter]->value()));
        break;
    }
}

void EditTool::createNew()
{
    isNew = true;
    groupBox->setEnabled(true);
}
