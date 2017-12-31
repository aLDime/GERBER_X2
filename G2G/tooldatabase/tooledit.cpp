#include "tooledit.h"

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
#include <QDebug>

int id = qRegisterMetaType<Tool>("Tool");

ToolEdit::ToolEdit(QWidget* parent)
    : QWidget(parent)
    , dsbList({ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr })
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
    on_cbxToolType_currentIndexChanged(0);

     //    setMaximumSize(QSize(393, 577));
}

ToolEdit::~ToolEdit()
{
}

void ToolEdit::on_cbxToolType_currentIndexChanged(int index)
{
    for (QDoubleSpinBox* dsb : dsbList) {
        if (dsb)
            dsb->setEnabled(true);
    }

    if (index != lastType && QMessageBox::question(this, "!!!", "Yes?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No) {
        cbxToolType->setCurrentIndex(lastType);
        return;
    }

    lastType = cbxToolType->currentIndex();

    switch (index) {
    case EndMill:
        dsbList[ClearencePassStepover]->setEnabled(false);
        dsbList[ClearencePassStepover]->setValue(0);
        dsbList[ClearencePassStepoverPercent]->setEnabled(false);
        dsbList[ClearencePassStepoverPercent]->setValue(0);
        //        dsbList[FlatDiameter]->setEnabled(false);
        //        dsbList[FlatDiameter]->setValue(0);
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
        //        dsbList[FlatDiameter]->setEnabled(false);
        //        dsbList[FlatDiameter]->setValue(0);
        dsbList[Stepover]->setEnabled(false);
        dsbList[Stepover]->setValue(0);
        dsbList[StepoverPercent]->setEnabled(false);
        dsbList[StepoverPercent]->setValue(0);
        dsbList[SideAngle]->setValue(120);
        lblPixmap->setPixmap(QPixmap(QString::fromUtf8(":/tool/drill.png")));
        break;
    default:
        break;
    }
    updateName();
}

Tool ToolEdit::getTool() const
{
    return tool;
}

void ToolEdit::apply()
{
    int i = 0;
    for (QDoubleSpinBox* dsb : dsbList) {
        if (dsb)
            tool.data.Params[i] = dsb->value();
        ++i;
    }

    tool.data.feedSpeeds = cbxFeedSpeeds->currentIndex();
    tool.data.spindleSpeed = sbSpindleSpeed->value();
    tool.data.toolType = static_cast<ToolType>(cbxToolType->currentIndex());
    tool.name = leName->text();
    tool.note = pteNote->document()->toRawText();
    emit toolEdited(tool);
}

void ToolEdit::setTool(const Tool& value)
{
    tool = value;
    for (int i = 0; i < dsbList.size(); ++i) {
        if (dsbList[i])
            dsbList[i]->setValue(tool.data.Params[i]);
    }

    cbxFeedSpeeds->setCurrentIndex(tool.data.feedSpeeds);
    sbSpindleSpeed->setValue(tool.data.spindleSpeed);
    lastType = tool.data.toolType;
    cbxToolType->setCurrentIndex(tool.data.toolType);
    leName->setText(tool.name);
    pteNote->document()->setPlainText(tool.note);

    bool fl = tool.data.toolType != Group;
    grBox_2->setVisible(fl);
    grBox_3->setVisible(fl);
    grBox_4->setVisible(fl);
    cbxToolType->setVisible(fl);
    label_6->setVisible(fl);
    lblPixmap->setVisible(fl);
}

void ToolEdit::setupUi(QWidget* ToolEdit)
{
    if (ToolEdit->objectName().isEmpty())
        ToolEdit->setObjectName(QStringLiteral("ToolEdit"));

    QFont font1;
    font1.setBold(false);
    font1.setWeight(50);
    ToolEdit->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));

    groupBox = new QGroupBox(ToolEdit);
    groupBox->setObjectName(QStringLiteral("groupBox"));

    label_5 = new QLabel(groupBox);
    label_5->setObjectName(QStringLiteral("label_5"));
    label_5->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    leName = new QLineEdit(groupBox);
    leName->setObjectName(QStringLiteral("leName"));

    label_6 = new QLabel(groupBox);
    label_6->setObjectName(QStringLiteral("label_6"));
    label_6->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    cbxToolType = new QComboBox(groupBox);
    cbxToolType->setObjectName(QStringLiteral("cbxToolType"));

    label_7 = new QLabel(groupBox);
    label_7->setObjectName(QStringLiteral("label_7"));
    label_7->setAlignment(Qt::AlignRight | Qt::AlignTop | Qt::AlignTrailing);

    pteNote = new QPlainTextEdit(groupBox);
    pteNote->setObjectName(QStringLiteral("pteNote"));
    pteNote->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));

    label_14 = new QLabel(groupBox);
    label_14->setObjectName(QStringLiteral("label_14"));
    label_14->setEnabled(true);
    label_14->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    cbxUnits = new QComboBox(groupBox);
    cbxUnits->setObjectName(QStringLiteral("cbxUnits"));
    cbxUnits->setEnabled(false);

    grBox_2 = new QGroupBox(groupBox);
    grBox_2->setObjectName(QStringLiteral("groupBox_2"));

    grBox_2->setFlat(false);
    label_8 = new QLabel(grBox_2);
    label_8->setObjectName(QStringLiteral("label_8"));

    label_8->setFont(font1);
    label_8->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    label_9 = new QLabel(grBox_2);
    label_9->setObjectName(QStringLiteral("label_9"));
    label_9->setFont(font1);
    label_9->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    //    label_10 = new QLabel(grBox_2);
    //    label_10->setObjectName(QStringLiteral("label_10"));
    //    label_10->setFont(font1);
    //    label_10->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    dsbList[Diameter] = new QDoubleSpinBox(grBox_2);
    dsbList[Diameter]->setObjectName(QStringLiteral("Diameter"));
    dsbList[Diameter]->setFont(font1);
    dsbList[Diameter]->setMaximum(100);
    dsbList[SideAngle] = new QDoubleSpinBox(grBox_2);
    dsbList[SideAngle]->setObjectName(QStringLiteral("SideAngle"));
    dsbList[SideAngle]->setFont(font1);
    dsbList[SideAngle]->setMaximum(180);
    //    dsbList[FlatDiameter] = new QDoubleSpinBox(grBox_2);
    //    dsbList[FlatDiameter]->setObjectName(QStringLiteral("FlatDiameter"));
    //    dsbList[FlatDiameter]->setFont(font1);
    //    dsbList[FlatDiameter]->setMaximum(100);

    QGridLayout* gridLayout_3 = new QGridLayout(grBox_2);
    gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
    gridLayout_3->setContentsMargins(6, 6, 6, 6);
    gridLayout_3->addWidget(label_8, 0, 0, 1, 1);
    gridLayout_3->addWidget(dsbList[Diameter], 0, 1, 1, 1);
    gridLayout_3->addWidget(label_9, 1, 0, 1, 1);
    gridLayout_3->addWidget(dsbList[SideAngle], 1, 1, 1, 1);
    //    gridLayout_3->addWidget(label_10, 2, 0, 1, 1);
    //    gridLayout_3->addWidget(dsbList[FlatDiameter], 2, 1, 1, 1);

    grBox_3 = new QGroupBox(groupBox);
    grBox_3->setObjectName(QStringLiteral("groupBox_3"));

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

    pbApply = new QPushButton(groupBox);
    pbApply->setObjectName(QStringLiteral("bpApply"));
    QSizePolicy sizePolicy4(QSizePolicy::Minimum, QSizePolicy::Minimum);
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(pbApply->sizePolicy().hasHeightForWidth());
    pbApply->setSizePolicy(sizePolicy4);

    lblPixmap = new QLabel(groupBox);
    lblPixmap->setObjectName(QStringLiteral("label"));
    lblPixmap->setPixmap(QPixmap(QString::fromUtf8(":/tool/endmill.png")));
    lblPixmap->setAlignment(Qt::AlignCenter);

    QGridLayout* gridLayout_4 = new QGridLayout(groupBox);
    gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
    gridLayout_4->addWidget(label_5,
        0, 0, 1, 1);
    gridLayout_4->addWidget(leName,
        0, 1, 1, 1);
    gridLayout_4->addWidget(label_6,
        1, 0, 1, 1);
    gridLayout_4->addWidget(cbxToolType,
        1, 1, 1, 1);
    gridLayout_4->addWidget(label_7,
        2, 0, 1, 1);
    gridLayout_4->addWidget(pteNote,
        2, 1, 1, 1);
    gridLayout_4->addWidget(label_14,
        3, 0, 1, 1);
    gridLayout_4->addWidget(cbxUnits,
        3, 1, 1, 1);
    gridLayout_4->addWidget(grBox_2,
        4, 0, 1, 2);
    gridLayout_4->addWidget(grBox_3,
        5, 0, 1, 2);
    gridLayout_4->addWidget(grBox_4,
        6, 0, 1, 2);
    gridLayout_4->addWidget(pbApply,
        6, 2, 1, 1);
    gridLayout_4->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding),
        7, 1, 1, 1);
    gridLayout_4->addWidget(lblPixmap,
        0, 2, 6, 1);

    QVBoxLayout* verticalLayout = new QVBoxLayout(ToolEdit);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    verticalLayout->addWidget(groupBox);
    verticalLayout->setMargin(0);
    ///////////////////////////////////////////////////////////////////////

    dsbList[Diameter]->setSingleStep(0.1);
    //    dsbList[FlatDiameter]->setSingleStep(0.1);

    connect(dsbList[Diameter], static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double) { updateName(); });
    connect(dsbList[SideAngle], static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double) { updateName(); });
    //    connect(dsbList[FlatDiameter], static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double) { updateName(); });
    connect(pbApply, &QPushButton::clicked, this, &ToolEdit::apply);

    ///////////////////////////////////////////////////////////////////////
    retranslateUi(ToolEdit);
    QMetaObject::connectSlotsByName(ToolEdit);
}

void ToolEdit::retranslateUi(QWidget* ToolEdit)
{
    ToolEdit->setWindowTitle(tr("Dialog"));
    pbApply->setText(tr("Apply"));
    cbxFeedSpeeds->clear();
    cbxFeedSpeeds->insertItems(0, QStringList() << tr("mm/sec") << tr("mm/min") << tr("m/min"));
    cbxUnits->clear();
    cbxUnits->insertItems(0, QStringList() << tr("mm") << tr("inches"));

    dsbList[ClearencePassStepover]->setSuffix(tr(" mm"));
    dsbList[ClearencePassStepoverPercent]->setSuffix(tr(" %"));
    dsbList[Diameter]->setSuffix(tr(" mm"));
    //    dsbList[FlatDiameter]->setSuffix(tr(" mm"));
    dsbList[PassDepth]->setSuffix(tr(" mm"));
    dsbList[SideAngle]->setSuffix(tr(" \302\260"));
    dsbList[Stepover]->setSuffix(tr(" mm"));
    dsbList[StepoverPercent]->setSuffix(tr(" %"));

    grBox_2->setTitle(tr("Geometry"));
    grBox_3->setTitle(tr("Cutting Parameters"));
    grBox_4->setTitle(tr("Feedp Speeds"));
    groupBox->setTitle(tr("Tool Info"));
    lblPixmap->setText(QString());
    //    label_10->setText(tr("Flat Diameter"));
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
    //    groupBox->setEnabled(false);
}

void ToolEdit::updateName()
{
    if (!isNew)
        return;
    switch (cbxToolType->currentIndex()) {
    case EndMill:
        leName->setText(QString(tr("End Mill (%1 mm)")).arg(dsbList[Diameter]->value()));
        break;
    case Engraving:
        leName->setText(QString(tr("Engrave (%2\302\260 %1 mm Tip Dia)")).arg(dsbList[/*FlatDiameter*/ Diameter]->value()).arg(dsbList[SideAngle]->value()));
        break;
    case Drill:
        leName->setText(QString(tr("Drill (%1 mm)")).arg(dsbList[Diameter]->value()));
        break;
    }
}

void ToolEdit::showEvent(QShowEvent* /*event*/)
{
    bool fl = tool.data.toolType != Group;
    grBox_2->setVisible(fl);
    grBox_3->setVisible(fl);
    grBox_4->setVisible(fl);
    cbxToolType->setVisible(fl);
    label_6->setVisible(fl);
    lblPixmap->setVisible(fl);
    //    setMinimumHeight(height());
    //    grBox_2->setVisible(false);
    //    grBox_3->setVisible(false);
    //    grBox_4->setVisible(false);
    //    cbxToolType->setVisible(false);
    //    label_6->setVisible(false);
    //    lblPixmap->setVisible(false);
}
/////////////////////////////////////////////
/// \brief EditToolDialog::EditToolDialog
/// \param parent
///
EditToolDialog::EditToolDialog(QWidget* parent, const Tool& tool)
    : QDialog(parent)
{
    setupUi(this);
    toolEdit->setTool(tool);
    toolEdit->cbxToolType->setEnabled(false);
}

EditToolDialog::~EditToolDialog() {}

void EditToolDialog::setupUi(QDialog* Dialog)
{
    if (Dialog->objectName().isEmpty())
        Dialog->setObjectName(QStringLiteral("Dialog"));
    Dialog->resize(298, 340);
    QVBoxLayout* verticalLayout = new QVBoxLayout(Dialog);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    toolEdit = new ToolEdit(Dialog);
    toolEdit->setObjectName(QStringLiteral("widget"));

    verticalLayout->addWidget(toolEdit);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(Dialog);
    buttonBox->setObjectName(QStringLiteral("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

    verticalLayout->addWidget(buttonBox);

    retranslateUi(Dialog);
    QObject::connect(toolEdit->pbApply, &QPushButton::clicked, [&] {
        //toolEdit->apply();
        accept();
    });
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, [&] {
        toolEdit->apply();
        accept();
    });
    QObject::connect(buttonBox, SIGNAL(rejected()), Dialog, SLOT(reject()));

    QMetaObject::connectSlotsByName(Dialog);
}

void EditToolDialog::retranslateUi(QDialog* Dialog)
{
    Dialog->setWindowTitle(QApplication::translate("Dialog", "Dialog", Q_NULLPTR));
}

Tool EditToolDialog::getTool() const
{
    return toolEdit->getTool();
}
