#include "profiletoolpathform.h"
#include "toolpathnameform.h"
#include "mainwindow.h"
#include "toolpathcreator.h"
#include "graphicsview/mygraphicsscene.h"

#include <QApplication>
#include <QDebug>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalMapper>
#include <QTabBar>

#include <QMessageBox>

#include <gerberfileholder.h>

#include <gerber/gerbergraphicsitem.h>

Q_DECLARE_METATYPE(QGraphicsItemGroup*)

ProfileToolpathForm::ProfileToolpathForm(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);

    rbOutside->setChecked(true);
    side = OUTSIDE;
    rbClimb->setChecked(true);
    direction = CLIMB;
    for (int i = 0; i < rbList.size(); ++i) {
        connect(rbList[i], &QRadioButton::clicked, [=]() {
            QStringList list = {
                ":/toolpath/outside_climb.png",
                ":/toolpath/inside_climb.png",
                ":/toolpath/on_climb.png",
                ":/toolpath/outside_conventional.png",
                ":/toolpath/inside_conventional.png",
                ":/toolpath/on_conventional.png"
            };
            QStringList name = { "PROFILE OUTSIDE", "PROFILE INSIDE", "PROFILE ON" };
            switch (i) {
            case OUTSIDE:
            case INSIDE:
            case ON:
                nameForm->setText(name[i]);
                side = i;
                break;
            case CLIMB:
            case CONVENTIONAL:
                direction = i;
                break;
            default:
                break;
            }
            lblPixmap->setPixmap(QPixmap(list[side + ((direction - CLIMB) * CLIMB)]));
        });
    }
    rbList[0]->clicked();
}

ProfileToolpathForm::~ProfileToolpathForm()
{
    qDebug() << "~ProfileToolpathForm";
}

void ProfileToolpathForm::changeEvent(QEvent* e)
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

void ProfileToolpathForm::showEvent(QShowEvent*)
{
    for (QWidget* widget : QApplication::topLevelWidgets()) {
        if (widget->objectName() == "MainWindow")
            for (QTabBar* tabBar : widget->findChildren<QTabBar*>()) {
                for (int i = 0; i < tabBar->count(); ++i) {
                    if (tabBar->tabText(i) == "Profile")
                        tabBar->setCurrentIndex(i);
                }
            }
    }
}

void ProfileToolpathForm::setupUi(QWidget* Form)
{
    if (Form->objectName().isEmpty())
        Form->setObjectName(QStringLiteral("ProfileToolpathForm"));
    Form->resize(260, 542);

    groupBox_1 = new QGroupBox(Form);
    groupBox_1->setObjectName(QStringLiteral("groupBox"));
    //    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    //    sizePolicy.setHorizontalStretch(0);
    //    sizePolicy.setVerticalStretch(0);
    //    sizePolicy.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
    //    groupBox->setSizePolicy(sizePolicy);
    QVBoxLayout* vLayout = new QVBoxLayout(groupBox_1);
    vLayout->setObjectName(QStringLiteral("horizontalLayout"));
    vLayout->setMargin(6);
    label = new QLabel(groupBox_1);
    label->setObjectName(QStringLiteral("label"));

    vLayout->addWidget(label);

    groupBox_2 = new QGroupBox(Form);
    groupBox_2->setObjectName(QStringLiteral("groupBox_2"));

    QFrame* frame_1 = new QFrame(groupBox_2);
    frame_1->setObjectName(QStringLiteral("frame_1"));
    frame_1->setFrameShape(QFrame::NoFrame);
    frame_1->setFrameShadow(QFrame::Plain);

    rbOutside = new QRadioButton(frame_1);
    rbOutside->setObjectName(QStringLiteral("rbOutside"));
    rbInside = new QRadioButton(frame_1);
    rbInside->setObjectName(QStringLiteral("rbInside"));
    rbOn = new QRadioButton(frame_1);
    rbOn->setObjectName(QStringLiteral("rbOn"));

    QVBoxLayout* verticalLayout_3 = new QVBoxLayout(frame_1);
    verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
    verticalLayout_3->setContentsMargins(0, 0, 0, 0);
    verticalLayout_3->addWidget(rbOutside);
    verticalLayout_3->addWidget(rbInside);
    verticalLayout_3->addWidget(rbOn);

    QFrame* frame_2 = new QFrame(groupBox_2);
    frame_2->setObjectName(QStringLiteral("frame_2"));
    frame_2->setFrameShape(QFrame::NoFrame);
    frame_2->setFrameShadow(QFrame::Plain);

    rbClimb = new QRadioButton(frame_2);
    rbClimb->setObjectName(QStringLiteral("rbClimb"));
    rbConventional = new QRadioButton(frame_2);
    rbConventional->setObjectName(QStringLiteral("rbConventional"));

    QVBoxLayout* verticalLayout_2 = new QVBoxLayout(frame_2);
    verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
    verticalLayout_2->setContentsMargins(0, 0, 0, 0);
    verticalLayout_2->addWidget(rbClimb);
    verticalLayout_2->addWidget(rbConventional);

    label_2 = new QLabel(groupBox_2);
    label_2->setObjectName(QStringLiteral("label_2"));
    label_2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    label_3 = new QLabel(groupBox_2);
    label_3->setObjectName(QStringLiteral("label_3"));
    label_3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    label_5 = new QLabel(groupBox_2);
    label_5->setObjectName(QStringLiteral("label_5"));

    dsbDepth = new QDoubleSpinBox(groupBox_2);
    dsbDepth->setObjectName(QStringLiteral("doubleSpinBox"));
    dsbDepth->setDecimals(3);
    dsbDepth->setMaximum(100);
    dsbDepth->setSingleStep(0.05);

    QFrame* line_2 = new QFrame(groupBox_2);
    line_2->setObjectName(QStringLiteral("line_2"));
    line_2->setFrameShape(QFrame::HLine);
    line_2->setFrameShadow(QFrame::Sunken);

    QFrame* line = new QFrame(groupBox_2);
    line->setObjectName(QStringLiteral("line"));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    lblToolName = new QLabel(groupBox_2);
    lblToolName->setObjectName(QStringLiteral("label_4"));

    lblPixmap = new QLabel(groupBox_2);
    lblPixmap->setObjectName(QStringLiteral("label_14"));
    QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(lblPixmap->sizePolicy().hasHeightForWidth());
    lblPixmap->setSizePolicy(sizePolicy1);

    label_6 = new QLabel(groupBox_2);
    label_6->setObjectName(QStringLiteral("label_6"));
    label_6->setAlignment(Qt::AlignRight | Qt::AlignTop);

    label_15 = new QLabel(groupBox_2);
    label_15->setObjectName(QStringLiteral("label_15"));
    label_15->setAlignment(Qt::AlignRight | Qt::AlignTop);

    pbSelectTool = new QPushButton(groupBox_2);
    pbSelectTool->setObjectName(QStringLiteral("pushButton_3"));

    pbEditTool = new QPushButton(groupBox_2);
    pbEditTool->setObjectName(QStringLiteral("pushButton_4"));

    QGridLayout* gridLayout = new QGridLayout(groupBox_2);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));
    gridLayout->setMargin(6);
    //    gridLayout->addWidget(frame_1, 9, 2, 1, 1);
    //    gridLayout->addWidget(frame_2, 11, 2, 1, 1);
    //    gridLayout->addWidget(label_2, 1, 0, 1, 1);
    //    gridLayout->addWidget(label_3, 3, 0, 1, 1);
    //    gridLayout->addWidget(label_5, 6, 0, 1, 4);
    //    gridLayout->addWidget(dsbDepth, 1, 2, 1, 2);
    //    gridLayout->addWidget(line_2, 5, 0, 1, 4);
    //    gridLayout->addWidget(line, 2, 0, 1, 4);
    //    gridLayout->addWidget(lblToolName, 3, 2, 1, 2);
    //    gridLayout->addWidget(lblPixmap, 9, 3, 3, 1);
    //    gridLayout->addWidget(label_6, 11, 0, 1, 1);
    //    gridLayout->addWidget(label_15, 9, 0, 1, 1);
    //    gridLayout->addWidget(pbSelectTool, 4, 3, 1, 1);
    //    gridLayout->addWidget(pbEditTool, 4, 2, 1, 1);

    gridLayout->addWidget(frame_1, 9, 1, 1, 1);
    gridLayout->addWidget(frame_2, 11, 1, 1, 1);
    gridLayout->addWidget(label_2, 1, 0, 1, 1);
    gridLayout->addWidget(label_3, 3, 0, 1, 1);
    gridLayout->addWidget(label_5, 6, 0, 1, 3);
    gridLayout->addWidget(dsbDepth, 1, 1, 1, 2);
    gridLayout->addWidget(line_2, 5, 0, 1, 3);
    gridLayout->addWidget(line, 2, 0, 1, 3);
    gridLayout->addWidget(lblToolName, 3, 1, 1, 2);
    gridLayout->addWidget(lblPixmap, 9, 2, 3, 1);
    gridLayout->addWidget(label_6, 11, 0, 1, 1);
    gridLayout->addWidget(label_15, 9, 0, 1, 1);
    gridLayout->addWidget(pbSelectTool, 4, 2, 1, 1);
    gridLayout->addWidget(pbEditTool, 4, 1, 1, 1);

    //    groupBox_3 = new QGroupBox(Form);
    //    groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
    //    groupBox_3->setEnabled(false);
    //    groupBox_3->setCheckable(true);
    //    label_7 = new QLabel(groupBox_3);
    //    label_7->setObjectName(QStringLiteral("label_7"));

    //    doubleSpinBox_2 = new QDoubleSpinBox(groupBox_3);
    //    doubleSpinBox_2->setObjectName(QStringLiteral("doubleSpinBox_2"));
    //    QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Fixed);
    //    sizePolicy2.setHorizontalStretch(0);
    //    sizePolicy2.setVerticalStretch(0);
    //    sizePolicy2.setHeightForWidth(doubleSpinBox_2->sizePolicy().hasHeightForWidth());
    //    doubleSpinBox_2->setSizePolicy(sizePolicy2);
    //    doubleSpinBox_2->setMaximum(100);

    //    label_8 = new QLabel(groupBox_3);
    //    label_8->setObjectName(QStringLiteral("label_8"));

    //    doubleSpinBox_3 = new QDoubleSpinBox(groupBox_3);
    //    doubleSpinBox_3->setObjectName(QStringLiteral("doubleSpinBox_3"));
    //    sizePolicy2.setHeightForWidth(doubleSpinBox_3->sizePolicy().hasHeightForWidth());
    //    doubleSpinBox_3->setSizePolicy(sizePolicy2);
    //    doubleSpinBox_3->setMaximum(100);

    //    pushButton_2 = new QPushButton(groupBox_3);
    //    pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
    //    groupBox_3->setChecked(false);

    //    QGridLayout* gridLayout_2 = new QGridLayout(groupBox_3);
    //    gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
    //    gridLayout_2->setMargin(6);
    //    gridLayout_2->addWidget(label_7, 0, 0, 1, 1);
    //    gridLayout_2->addWidget(doubleSpinBox_2, 0, 1, 1, 1);
    //    gridLayout_2->addWidget(label_8, 1, 0, 1, 1);
    //    gridLayout_2->addWidget(doubleSpinBox_3, 1, 1, 1, 1);
    //    gridLayout_2->addWidget(pushButton_2, 2, 0, 1, 2);

    groupBox_4 = new QGroupBox(Form);
    groupBox_4->setObjectName(QStringLiteral("groupBox_4"));
    nameForm = new ToolpathNameForm(groupBox_4);
    nameForm->setObjectName(QStringLiteral("nameForm"));

    QVBoxLayout* vLayout_1 = new QVBoxLayout(groupBox_4);
    vLayout_1->setObjectName(QStringLiteral("gridLayout_3"));
    vLayout_1->setMargin(6);
    vLayout_1->addWidget(nameForm);

    pbCalculate = new QPushButton(Form);
    pbCalculate->setObjectName(QStringLiteral("pushButton"));
    connect(pbCalculate, &QPushButton::clicked, this, &ProfileToolpathForm::calculate);

    pbClose = new QPushButton(Form);
    pbClose->setObjectName(QStringLiteral("pbClose"));
    connect(pbClose, &QPushButton::clicked, [=]() { if (static_cast<QDockWidget*>(parent())) static_cast<QDockWidget*>(parent())->hide(); });

    QVBoxLayout* verticalLayout = new QVBoxLayout(Form);
    verticalLayout->setSpacing(6);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    verticalLayout->setMargin(6);
    verticalLayout->addWidget(groupBox_1);
    verticalLayout->addWidget(groupBox_2);
    //    verticalLayout->addWidget(groupBox_3);
    verticalLayout->addWidget(groupBox_4);
    verticalLayout->addWidget(pbCalculate);
    verticalLayout->addWidget(pbClose);
    verticalLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    rbList = (QList<QRadioButton*>() = { rbOutside, rbInside, rbOn, rbClimb, rbConventional });

    //    QString style1 = "QGroupBox {"
    //                     "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #8080FF, stop: 1 #FFFFFF);"
    //                     "border: 1px solid gray;"
    //                     "border-radius: 5px;"
    //                     //                     "margin-top: 1ex;"
    //                     "}";

    //    QString style2 = "QGroupBox {"
    //                     "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF);"
    //                     "border: 1px solid gray;"
    //                     "border-radius: 5px;"
    //                     //                     "margin-top: 1ex;"
    //                     "}";

    //    groupBox_1->setStyleSheet(style1);
    //    groupBox_2->setStyleSheet(style2);
    //    //    groupBox_3->setStyleSheet(style2);
    //    groupBox_4->setStyleSheet(style2);

    retranslateUi(Form);

    QMetaObject::connectSlotsByName(Form);
}

void ProfileToolpathForm::retranslateUi(QWidget* Form)
{
    Form->setWindowTitle(tr("Profile Toolpath"));
    //    doubleSpinBox_2->setSuffix(tr(" mm"));
    //    doubleSpinBox_3->setSuffix(tr(" mm"));
    //    groupBox_3->setTitle(tr("Tabs"));
    //    label_7->setText(tr("Lenght:"));
    //    label_8->setText(tr("Ticknes:"));
    //    pushButton_2->setText(tr("Edit Tabs..."));
    dsbDepth->setSuffix(tr(" mm"));
    groupBox_1->setTitle(QString());
    groupBox_2->setTitle(QString());
    groupBox_4->setTitle(QString());
    label->setText(tr("Profile Toolpath"));
    label_15->setText(tr("Side:"));
    label_2->setText(tr("Cut Depth:"));
    label_3->setText(tr("Tool:"));
    label_5->setText(tr("Machine Vectors..."));
    label_6->setText(tr("Direction:"));
    lblToolName->setText(tr("Tool Name"));
    pbCalculate->setText(tr("Calculate"));
    pbClose->setText(tr("Close"));
    pbEditTool->setText(tr("Edit..."));
    pbSelectTool->setText(tr("Select..."));
    rbClimb->setText(tr("Climb"));
    rbConventional->setText(tr("Conventional"));
    rbInside->setText(tr("Inside"));
    rbOn->setText(tr("On"));
    rbOutside->setText(tr("Outside"));
}

void ProfileToolpathForm::calculate()
{
    MainWindow* mw = static_cast<MainWindow*>(parentWidget()->parentWidget());
    QGraphicsScene* csene = mw->graphicsView->scene();

    QList<QGraphicsItem*> wItems(csene->selectedItems());

    if (wItems.isEmpty() || qFuzzyIsNull(dsbDepth->value())) {
        QMessageBox::warning(this, "!!!", tr("No selected gerber file"));
        return;
    }

    Paths value;
    for (QGraphicsItem* item : wItems) {
        value.append(static_cast<GerberWorkItem*>(item)->getPaths());
    }

    ToolPathCreator tpc;
    Paths paths(tpc.setPaths(value).ToolPathProfile(static_cast<MILLING>(side), dsbDepth->value()));

    QPainterPath painterPath;
    QList<QGraphicsItem*> items;
    QGraphicsPathItem* pathItem;

    Paths paths2;
    ClipperOffset offset;
    for (Path& path : paths) {
        offset.Clear();
        offset.AddPath(path, jtRound, etClosedLine);
        offset.Execute(paths2, dsbDepth->value() * 0.5 * uScale);
        painterPath = QPainterPath();
        for (Path& path : paths2) {
            painterPath.addPolygon(PathToQPolygon(path));
        }
        pathItem = new QGraphicsPathItem(painterPath);
        pathItem->setPen(Qt::NoPen);
        pathItem->setBrush(QColor(255, 255, 255, 50));
        items.append(pathItem);
    }

    painterPath = QPainterPath();
    for (Path& path : paths) {
        painterPath.addPolygon(PathToQPolygon(path));
    }
    pathItem = new QGraphicsPathItem(painterPath);
    pathItem->setPen(QPen(Qt::white, 0.0));
    pathItem->setBrush(Qt::NoBrush);
    items.append(pathItem);

    QGraphicsItemGroup* group = csene->createItemGroup(items);
    group->setAcceptHoverEvents(false);
    group->setAcceptTouchEvents(false);
    group->setAcceptedMouseButtons(Qt::NoButton);

//    mw->treeWidget->addMilling(nameForm->text(), group);
}
