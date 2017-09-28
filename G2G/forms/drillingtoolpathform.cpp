#include "drillingtoolpathform.h"
#include "toolpathnameform.h"

#include <QApplication>
#include <QCheckBox>
#include <QDebug>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QTabBar>

DrillingToolpathForm::DrillingToolpathForm(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);
}

DrillingToolpathForm::~DrillingToolpathForm()
{
    qDebug() << "~DrillingToolpathForm";
}

void DrillingToolpathForm::changeEvent(QEvent* e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi(this);
        break;
    default:
        break;
    }
}

//void DrillingToolpathForm::showEvent(QShowEvent*)
//{
//    for (QWidget* widget : QApplication::topLevelWidgets()) {
//        if (widget->objectName() == "MainWindow") {
//            for (QTabBar* tabBar : widget->findChildren<QTabBar*>()) {
//                for (int i = 0; i < tabBar->count(); ++i) {
//                    if (tabBar->tabText(i) == "Drilling")
//                        tabBar->setCurrentIndex(i);
//                }
//            }
//        }
//    }
//}

void DrillingToolpathForm::setupUi(QWidget* Form)
{
    if (Form->objectName().isEmpty())
        Form->setObjectName(QStringLiteral("Form"));
    Form->resize(262, 562);

    groupBox_1 = new QGroupBox(Form);
    groupBox_1->setObjectName(QStringLiteral("groupBox"));
    label_2 = new QLabel(groupBox_1);
    label_2->setObjectName(QStringLiteral("label_2"));

    QVBoxLayout* verticalLayout_3 = new QVBoxLayout(groupBox_1);
    verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
    verticalLayout_3->setMargin(6);
    verticalLayout_3->addWidget(label_2);

    groupBox_2 = new QGroupBox(Form);
    groupBox_2->setObjectName(QStringLiteral("groupBox_2"));

    chbPeckDrilling = new QCheckBox(groupBox_2);
    chbPeckDrilling->setObjectName(QStringLiteral("chbPeckDrilling"));
    chbPeckDrilling->setEnabled(false);

    label = new QLabel(groupBox_2);
    label->setObjectName(QStringLiteral("label"));
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    label_3 = new QLabel(groupBox_2);
    label_3->setObjectName(QStringLiteral("label_3"));
    label_3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    pbEdit = new QPushButton(groupBox_2);
    pbEdit->setObjectName(QStringLiteral("pbEdit"));

    pbSelect = new QPushButton(groupBox_2);
    pbSelect->setObjectName(QStringLiteral("pbSelect"));

    QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(pbEdit->sizePolicy().hasHeightForWidth());
    pbEdit->setSizePolicy(sizePolicy);
    sizePolicy.setHeightForWidth(pbSelect->sizePolicy().hasHeightForWidth());
    pbSelect->setSizePolicy(sizePolicy);

    label_4 = new QLabel(groupBox_2);
    label_4->setObjectName(QStringLiteral("label_4"));

    dsbDepth = new QDoubleSpinBox(groupBox_2);
    dsbDepth->setObjectName(QStringLiteral("dsbDepth"));
    dsbDepth->setMaximum(1000);

    label_5 = new QLabel(groupBox_2);
    label_5->setObjectName(QStringLiteral("label_5"));
    label_5->setEnabled(false);
    label_5->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    dsbGap = new QDoubleSpinBox(groupBox_2);
    dsbGap->setObjectName(QStringLiteral("dsbGap"));
    dsbGap->setEnabled(false);

    line_2 = new QFrame(groupBox_2);
    line_2->setObjectName(QStringLiteral("line_2"));
    line_2->setFrameShape(QFrame::HLine);
    line_2->setFrameShadow(QFrame::Sunken);

    line = new QFrame(groupBox_2);
    line->setObjectName(QStringLiteral("line"));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    QGridLayout* gridLayout_2 = new QGridLayout(groupBox_2);
    gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
    gridLayout_2->setMargin(6);
    gridLayout_2->addWidget(chbPeckDrilling, 6, 0, 1, 2);
    gridLayout_2->addWidget(label, 0, 0, 1, 1);
    gridLayout_2->addWidget(label_3, 2, 0, 1, 1);
    gridLayout_2->addWidget(pbEdit, 3, 1, 1, 1);
    gridLayout_2->addWidget(pbSelect, 3, 2, 1, 1);
    gridLayout_2->addWidget(label_4, 2, 1, 1, 2);
    gridLayout_2->addWidget(dsbDepth, 0, 1, 1, 2);
    gridLayout_2->addWidget(label_5, 7, 0, 1, 1);
    gridLayout_2->addWidget(dsbGap, 7, 1, 1, 2);
    gridLayout_2->addWidget(line_2, 5, 0, 1, 3);
    gridLayout_2->addWidget(line, 1, 0, 1, 3);

    groupBox_3 = new QGroupBox(Form);
    groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
    QVBoxLayout* verticalLayout = new QVBoxLayout(groupBox_3);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    verticalLayout->addWidget(new ToolpathNameForm(groupBox_3));

    pbCalculate = new QPushButton(Form);
    pbCalculate->setObjectName(QStringLiteral("pbCalculate"));

    pbClose = new QPushButton(Form);
    pbClose->setObjectName(QStringLiteral("pbClose"));
    connect(pbClose, &QPushButton::clicked, [=]() { if (static_cast<QDockWidget*>(parent())) static_cast<QDockWidget*>(parent())->hide(); });

    QVBoxLayout* verticalLayout_1 = new QVBoxLayout(Form);
    verticalLayout_1->setObjectName(QStringLiteral("gridLayout"));
    verticalLayout_1->setMargin(6);
    verticalLayout_1->addWidget(groupBox_1);
    verticalLayout_1->addWidget(groupBox_2);
    verticalLayout_1->addWidget(groupBox_3);
    verticalLayout_1->addWidget(pbCalculate);
    verticalLayout_1->addWidget(pbClose);
    verticalLayout_1->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    //    QString style1 = "QGroupBox {"
    //                     "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FF8080, stop: 1 #FFFFFF);"
    //                     "border: 1px solid gray;"
    //                     "border-radius: 5px;"
    ////                     "margin-top: 1ex;"
    //                     "}";

    //    QString style2 = "QGroupBox {"
    //                     "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF);"
    //                     "border: 1px solid gray;"
    //                     "border-radius: 5px;"
    ////                     "margin-top: 1ex;"
    //                     "}";

    //    groupBox_1->setStyleSheet(style1);
    //    groupBox_2->setStyleSheet(style2);
    //    groupBox_3->setStyleSheet(style2);

    retranslateUi(Form);

    QMetaObject::connectSlotsByName(Form);
}

void DrillingToolpathForm::retranslateUi(QWidget* Form)
{
    Form->setWindowTitle(tr("Form"));
    chbPeckDrilling->setText(tr("Peck Drilling"));
    dsbDepth->setSuffix(tr(" mm"));
    dsbGap->setSuffix(tr(" mm"));
    groupBox_1->setTitle(QString());
    groupBox_2->setTitle(QString());
    groupBox_3->setTitle(QString());
    label->setText(tr("Cut Depth:"));
    label_2->setText(tr("Drilling Toolpath"));
    label_3->setText(tr("Tool:"));
    label_4->setText(tr("TextLabel"));
    label_5->setText(tr("Retract Gap:"));
    pbCalculate->setText(tr("Calculate"));
    pbClose->setText(tr("Close"));
    pbEdit->setText(tr("Edit..."));
    pbSelect->setText(tr("Select..."));
}
