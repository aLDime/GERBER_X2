#include "pocketwidget.h"
#include "profilewidget.h"
#include "toolpathwidget.h"
#include "toolpathwidget.h"

#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QDebug>

#include <QButtonGroup>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDockWidget>
#include <QSettings>

#include <tooldatabase/tooldatabase.h>
#include <tooldatabase/tooledit.h>

ToolPathWidget::ToolPathWidget(int type, QWidget* parent)
    : QWidget(parent)
    , type(type)
{
    setupUi(this);

    connect(pbSelectTool1, &QPushButton::clicked, [&] {
        ToolDatabase tdb(this, { EndMill, Engraving /*, Drill*/ });
        if (tdb.exec())
            tpcWidget->setTool(0, tdb.getTool());
    });

    connect(pbEditTool1, &QPushButton::clicked, [&] {
        EditToolDialog tdb(this, tpcWidget->getTool(0));
        if (tdb.exec())
            tpcWidget->setTool(0, tdb.getTool());
    });

    connect(pbSelectTool2, &QPushButton::clicked, [&] {
        ToolDatabase tdb(this, { EndMill, Engraving /*, Drill*/ });
        if (tdb.exec())
            tpcWidget->setTool(1, tdb.getTool());
    });

    connect(pbEditTool2, &QPushButton::clicked, [&] {
        EditToolDialog tdb(this, tpcWidget->getTool(1));
        if (tdb.exec())
            tpcWidget->setTool(1, tdb.getTool());
    });

    QSettings settings;
    settings.beginGroup(QString("ToolPathWidget%1").arg(type));
    dsbxDepth->setValue(settings.value("Depth").toDouble());
    settings.endGroup();

    setVisibleTool2(type == POCKET_TOOLPATH_FORM);
}

ToolPathWidget::~ToolPathWidget()
{
    QSettings settings;
    settings.beginGroup(QString("ToolPathWidget%1").arg(type));
    settings.setValue("Depth", dsbxDepth->value());
    settings.endGroup();
}

void ToolPathWidget::setVisibleTool2(bool visible)
{
    labelT2->setEnabled(false);
    lblToolName2->setEnabled(false);
    pbSelectTool2->setEnabled(false);
    pbEditTool2->setEnabled(false);

    cbxTool2->setVisible(visible);
    labelT2->setVisible(visible);
    lblToolName2->setVisible(visible);
    lineT2->setVisible(visible);
    pbEditTool2->setVisible(visible);
    pbSelectTool2->setVisible(visible);
}

void ToolPathWidget::setupUi(QWidget* Form)
{
    if (Form->objectName().isEmpty())
        Form->setObjectName(QStringLiteral("Form"));
    Form->resize(232, 655);
    // Form->setStyleSheet(QLatin1String("QGroupBox {\n"
    // "/*background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF);*/\n"
    // "background-color:rgb(255, 255, 255);\n"
    // "border: 1px solid gray;\n"
    // "border-radius: 5px;\n"
    // "/*margin-top: 0ex; /* leave space at the top for the title */\n"
    // "}"));
    verticalLayout = new QVBoxLayout(Form);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    verticalLayout->setContentsMargins(6, 6, 6, 6);
    groupBox = new QGroupBox(Form);
    groupBox->setObjectName(QStringLiteral("groupBox"));
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
    groupBox->setSizePolicy(sizePolicy);
    verticalLayout_2 = new QVBoxLayout(groupBox);
    verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
    verticalLayout_2->setContentsMargins(6, 6, 6, 6);
    //////////////////////////////////////////////////////////////
    lblPanelName = new QLabel(groupBox);
    lblPanelName->setObjectName(QStringLiteral("lblPanelName"));

    verticalLayout_2->addWidget(lblPanelName);
    verticalLayout->addWidget(groupBox);

    groupBox_2 = new QGroupBox(Form);
    groupBox_2->setObjectName(QStringLiteral("groupBox_2"));

    lineT2 = new QFrame(groupBox_2);
    lineT2->setObjectName(QStringLiteral("lineT2"));
    lineT2->setFrameShadow(QFrame::Plain);
    lineT2->setLineWidth(1);
    lineT2->setFrameShape(QFrame::HLine);

    pbSelectTool1 = new QPushButton(groupBox_2);
    pbSelectTool1->setObjectName(QStringLiteral("pbSelectTool1"));
    QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(pbSelectTool1->sizePolicy().hasHeightForWidth());
    pbSelectTool1->setSizePolicy(sizePolicy1);

    pbSelectTool2 = new QPushButton(groupBox_2);
    pbSelectTool2->setObjectName(QStringLiteral("pbSelectTool2"));

    cbxTool2 = new QCheckBox(groupBox_2);
    cbxTool2->setObjectName(QStringLiteral("cbxTool2"));
    QFont font;
    font.setBold(true);
    font.setWeight(75);
    cbxTool2->setFont(font);

    pbEditTool1 = new QPushButton(groupBox_2);
    pbEditTool1->setObjectName(QStringLiteral("pbEditTool1"));
    pbEditTool1->setSizePolicy(sizePolicy1);

    lblToolName1 = new QLabel(groupBox_2);
    lblToolName1->setObjectName(QStringLiteral("lblToolName1"));
    lblToolName1->setText(QApplication::translate("Form", "Select the tool", Q_NULLPTR));

    lblToolName2 = new QLabel(groupBox_2);
    lblToolName2->setObjectName(QStringLiteral("lblToolName2"));
    lblToolName2->setText(QApplication::translate("Form", "Select the tool", Q_NULLPTR));

    pbEditTool2 = new QPushButton(groupBox_2);
    pbEditTool2->setObjectName(QStringLiteral("pbEditTool2"));

    labelT1 = new QLabel(groupBox_2);
    labelT1->setObjectName(QStringLiteral("labelT1"));
    QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Preferred);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(labelT1->sizePolicy().hasHeightForWidth());
    labelT1->setSizePolicy(sizePolicy2);
    labelT1->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    labelT2 = new QLabel(groupBox_2);
    labelT2->setObjectName(QStringLiteral("labelT2"));
    labelT2->setSizePolicy(sizePolicy2);
    labelT2->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    lineT1 = new QFrame(groupBox_2);
    lineT1->setObjectName(QStringLiteral("lineT1"));
    lineT1->setFrameShadow(QFrame::Plain);
    lineT1->setLineWidth(1);
    lineT1->setFrameShape(QFrame::HLine);

    label_1 = new QLabel(groupBox_2);
    label_1->setObjectName(QStringLiteral("labelT1_2"));
    sizePolicy2.setHeightForWidth(label_1->sizePolicy().hasHeightForWidth());
    label_1->setSizePolicy(sizePolicy2);

    dsbxDepth = new QDoubleSpinBox(groupBox_2);
    dsbxDepth->setObjectName(QStringLiteral("dsbxDepth"));
    dsbxDepth->setButtonSymbols(QAbstractSpinBox::NoButtons);
    dsbxDepth->setDecimals(3);
    dsbxDepth->setMaximum(100);
    dsbxDepth->setSingleStep(0.01);

    gridLayout = new QGridLayout(groupBox_2);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));
    gridLayout->setContentsMargins(6, 6, 6, 6);
    gridLayout->addWidget(cbxTool2, 4, 0, 1, 3);
    gridLayout->addWidget(dsbxDepth, 0, 1, 1, 2);
    gridLayout->addWidget(label_1, 0, 0, 1, 1);
    gridLayout->addWidget(labelT1, 1, 0, 1, 1);
    gridLayout->addWidget(labelT2, 5, 0, 1, 1);
    gridLayout->addWidget(lblToolName1, 1, 1, 1, 2);
    gridLayout->addWidget(lblToolName2, 5, 1, 1, 2);
    gridLayout->addWidget(lineT1, 3, 0, 1, 3);
    gridLayout->addWidget(lineT2, 7, 0, 1, 3);
    gridLayout->addWidget(pbEditTool1, 2, 2, 1, 1);
    gridLayout->addWidget(pbEditTool2, 6, 2, 1, 1);
    gridLayout->addWidget(pbSelectTool1, 2, 1, 1, 1);
    gridLayout->addWidget(pbSelectTool2, 6, 1, 1, 1);

    //////////////////////////////////////////////////////////////
    verticalLayout->addWidget(groupBox_2);
    groupBox_3 = new QGroupBox(Form);
    groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
    sizePolicy.setHeightForWidth(groupBox_3->sizePolicy().hasHeightForWidth());
    groupBox_3->setSizePolicy(sizePolicy);
    formLayout = new QFormLayout(groupBox_3);
    formLayout->setObjectName(QStringLiteral("formLayout"));
    formLayout->setContentsMargins(6, 6, 6, 6);
    formLayout->setLabelAlignment(Qt::AlignRight);
    label = new QLabel(groupBox_3);
    label->setObjectName(QStringLiteral("label"));

    formLayout->setWidget(0, QFormLayout::LabelRole, label);

    lblSafeZ = new QLabel(groupBox_3);
    lblSafeZ->setObjectName(QStringLiteral("lblSafeZ"));

    formLayout->setWidget(0, QFormLayout::FieldRole, lblSafeZ);

    label_4 = new QLabel(groupBox_3);
    label_4->setObjectName(QStringLiteral("label_5"));
    label_4->setTextFormat(Qt::AutoText);

    formLayout->setWidget(1, QFormLayout::LabelRole, label_4);

    lblHome = new QLabel(groupBox_3);
    lblHome->setObjectName(QStringLiteral("lblHome"));

    formLayout->setWidget(1, QFormLayout::FieldRole, lblHome);

    label_5 = new QLabel(groupBox_3);
    label_5->setObjectName(QStringLiteral("label_8"));

    formLayout->setWidget(2, QFormLayout::LabelRole, label_5);

    lblZero = new QLabel(groupBox_3);
    lblZero->setObjectName(QStringLiteral("lblZero"));

    formLayout->setWidget(2, QFormLayout::FieldRole, lblZero);

    line_3 = new QFrame(groupBox_3);
    line_3->setObjectName(QStringLiteral("line_3"));
    line_3->setFrameShadow(QFrame::Plain);
    line_3->setLineWidth(1);
    line_3->setFrameShape(QFrame::HLine);

    formLayout->setWidget(3, QFormLayout::SpanningRole, line_3);

    label_3 = new QLabel(groupBox_3);
    label_3->setObjectName(QStringLiteral("label_10"));

    formLayout->setWidget(4, QFormLayout::LabelRole, label_3);

    lblVectors = new QLabel(groupBox_3);
    lblVectors->setObjectName(QStringLiteral("lblVectors"));

    formLayout->setWidget(4, QFormLayout::FieldRole, lblVectors);

    line_4 = new QFrame(groupBox_3);
    line_4->setObjectName(QStringLiteral("line_4"));
    line_4->setFrameShadow(QFrame::Plain);
    line_4->setLineWidth(1);
    line_4->setFrameShape(QFrame::HLine);

    formLayout->setWidget(5, QFormLayout::SpanningRole, line_4);

    label_2 = new QLabel(groupBox_3);
    label_2->setObjectName(QStringLiteral("label_2"));

    formLayout->setWidget(6, QFormLayout::LabelRole, label_2);

    leNameToolPath = new QLineEdit(groupBox_3);
    leNameToolPath->setObjectName(QStringLiteral("leNameToolPath"));

    formLayout->setWidget(6, QFormLayout::FieldRole, leNameToolPath);

    verticalLayout->addWidget(groupBox_3);

    pbCalculate = new QPushButton(Form);
    pbCalculate->setObjectName(QStringLiteral("pbCalculate"));

    verticalLayout->addWidget(pbCalculate);

    pbClose = new QPushButton(Form);
    pbClose->setObjectName(QStringLiteral("pbClose"));

    verticalLayout->addWidget(pbClose);

    verticalLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));

    //////////////////////////////////////////////////////////////
    switch (type) {
    case PROFILE_TOOLPATH_FORM:
        tpcWidget = new ProfileWidget(groupBox_2);
        break;
    case POCKET_TOOLPATH_FORM:
        tpcWidget = new PocketWidget(groupBox_2);
        break;
    case DRILLING_TOOLPATH_FORM:
        tpcWidget = new ProfileWidget(groupBox_2);
        break;
    }
    tpcWidget->setObjectName(QStringLiteral("tpcWidget"));
    gridLayout->addWidget(tpcWidget, 8, 0, 1, 3);
    //////////////////////////////////////////////////////////////

    retranslateUi(Form);

    QObject::connect(cbxTool2, SIGNAL(clicked(bool)), labelT2, SLOT(setEnabled(bool)));
    QObject::connect(cbxTool2, SIGNAL(clicked(bool)), lblToolName2, SLOT(setEnabled(bool)));
    QObject::connect(cbxTool2, SIGNAL(clicked(bool)), pbSelectTool2, SLOT(setEnabled(bool)));
    QObject::connect(cbxTool2, SIGNAL(clicked(bool)), pbEditTool2, SLOT(setEnabled(bool)));

    connect(pbClose, &QPushButton::clicked, [=]() { if (parent()) static_cast<QDockWidget*>(parent())->hide(); });
    connect(pbCalculate, &QPushButton::clicked, tpcWidget, &Widget::calculate);

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
    groupBox->setStyleSheet(styleSheet);
    groupBox_2->setStyleSheet(styleSheet);
    groupBox_3->setStyleSheet(styleSheet);

    QMetaObject::connectSlotsByName(Form);
}

void ToolPathWidget::retranslateUi(QWidget* Form)
{
    Form->setWindowTitle(QApplication::translate("Form", "Form", Q_NULLPTR));
    pbSelectTool1->setText(QApplication::translate("Form", "Select", Q_NULLPTR));
    pbSelectTool2->setText(QApplication::translate("Form", "Select", Q_NULLPTR));
    cbxTool2->setText(QApplication::translate("Form", "Use larger area clearance tool", Q_NULLPTR));
    pbEditTool1->setText(QApplication::translate("Form", "Edit", Q_NULLPTR));
    pbEditTool2->setText(QApplication::translate("Form", "Edit", Q_NULLPTR));
    labelT1->setText(QApplication::translate("Form", "<html><head/><body><p><span style=\"font-weight:600;\">Tool:</span></p></body></html>", Q_NULLPTR));
    labelT2->setText(QApplication::translate("Form", "<html><head/><body><p><span style=\"font-weight:600;\">Tool:</span></p></body></html>", Q_NULLPTR));
    label_1->setText(QApplication::translate("Form", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\"\"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
                                                     "<html><head><meta name=\"qrichtext\"content=\"1\"/><style type=\"text/css\">\n"
                                                     "p, li { white-space: pre-wrap; }\n"
                                                     "</style></head><body style=\"font-family:'MS Shell Dlg 2'; font-size:8pt; font-weight:400; font-style:normal;\">\n"
                                                     "<p style=\"margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\"font-weight:600;\">Cutting </span></p>\n"
                                                     "<p style=\"margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\"font-weight:600;\">Depth:</span></p></body></html>",
        Q_NULLPTR));
    dsbxDepth->setSuffix(QApplication::translate("Form", " mm", Q_NULLPTR));
    label->setText(QApplication::translate("Form", "Safe Z:", Q_NULLPTR));
    lblSafeZ->setText(QApplication::translate("Form", "TextLabel", Q_NULLPTR));
    label_4->setText(QApplication::translate("Form", "Home:", Q_NULLPTR));
    lblHome->setText(QApplication::translate("Form", "TextLabel", Q_NULLPTR));
    label_5->setText(QApplication::translate("Form", "Zero:", Q_NULLPTR));
    lblZero->setText(QApplication::translate("Form", "TextLabel", Q_NULLPTR));
    label_3->setText(QApplication::translate("Form", "Vectors:", Q_NULLPTR));
    lblVectors->setText(QApplication::translate("Form", "TextLabel", Q_NULLPTR));
    label_2->setText(QApplication::translate("Form", "<html><head/><body><p><span style=\"font-weight:600;\">Name:</span></p></body></html>", Q_NULLPTR));
    pbCalculate->setText(QApplication::translate("Form", "Calculate", Q_NULLPTR));
    pbClose->setText(QApplication::translate("Form", "Close", Q_NULLPTR));
}
