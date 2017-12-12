#include "pockettoolpathform.h"
#include "toolpathnameform.h"

#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalMapper>
#include <QSpacerItem>
#include <QTabBar>
#include <QVBoxLayout>
#include <QWidget>
#include <mainwindow.h>
#include <toolpathcreator.h>
#include <QtCore/QVariant>
#include <gerber/gerbergraphicsitem.h>

PocketToolpathForm::PocketToolpathForm(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);

    for (int i = 0; i < rbList.size(); ++i) {
        connect(rbList[i], &QRadioButton::clicked, [=]() {
            QStringList list = {
                ":/toolpath/offset_climb.png",
                ":/toolpath/raster_climb.png",
                ":/toolpath/offset_conventional.png",
                ":/toolpath/raster_conventional.png",
            };
            QStringList name = { "POCKET OFFSET", "POCKET RASTER" };
            switch (i) {
            case OFFSET:
            case RASTER:
                nameForm->setText(name[i]);
                type = i;
                break;
            case CLIMB:
            case CONVENTIONAL:
                direction = i;
                break;
            default:
                break;
            }
            lblPixmap->setPixmap(QPixmap(list[type + ((direction - CLIMB) * CLIMB)]));
        });
    }

    rbOffset->setChecked(true);
    type = OFFSET;
    rbClimb->setChecked(true);
    direction = CLIMB;
    rbList[0]->clicked(OFFSET);
}

PocketToolpathForm::~PocketToolpathForm()
{
    qDebug() << "~PocketToolpathForm";
}

void PocketToolpathForm::changeEvent(QEvent* e)
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

//void PocketToolpathForm::showEvent(QShowEvent*)
//{
//    for (QWidget* widget : QApplication::topLevelWidgets()) {
//        if (widget->objectName() == "MainWindow") {
//            for (QTabBar* tabBar : widget->findChildren<QTabBar*>()) {
//                for (int i = 0; i < tabBar->count(); ++i) {
//                    if (tabBar->tabText(i) == "Pocket")
//                        tabBar->setCurrentIndex(i);
//                }
//            }
//        }
//    }
//}

void PocketToolpathForm::setupUi(QWidget* Form)
{
    if (Form->objectName().isEmpty())
        Form->setObjectName(QStringLiteral("ProfileToolpathForm"));
    //groupBox
    groupBox_1 = new QGroupBox(Form);
    groupBox_1->setObjectName(QStringLiteral("groupBox_1"));
    //    groupBox->setBackgroundRole(QPalette::Text);
    label = new QLabel(groupBox_1);
    label->setObjectName(QStringLiteral("label"));
    QVBoxLayout* vLayout_1 = new QVBoxLayout(groupBox_1);
    vLayout_1->setMargin(6);
    vLayout_1->setObjectName(QStringLiteral("verticalLayout_2"));
    vLayout_1->addWidget(label);

    //groupBox_2
    groupBox_2 = new QGroupBox(Form);
    groupBox_2->setObjectName(QStringLiteral("groupBox_2"));

    QFrame* frame_1 = new QFrame(groupBox_2);
    frame_1->setObjectName(QStringLiteral("frame"));
    frame_1->setFrameShape(QFrame::NoFrame);
    frame_1->setFrameShadow(QFrame::Plain);

    rbOffset = new QRadioButton(frame_1);
    rbOffset->setObjectName(QStringLiteral("rbOffset"));
    rbRaster = new QRadioButton(frame_1);
    rbRaster->setObjectName(QStringLiteral("rbRaster"));
    rbRaster->setEnabled(false);

    QVBoxLayout* verticalLayout_3 = new QVBoxLayout(frame_1);
    verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
    verticalLayout_3->setContentsMargins(0, 0, 0, 0);
    verticalLayout_3->addWidget(rbOffset);
    verticalLayout_3->addWidget(rbRaster);

    QFrame* frame_2 = new QFrame(groupBox_2);
    frame_2->setObjectName(QStringLiteral("frame_2"));
    frame_2->setFrameShape(QFrame::NoFrame);
    frame_2->setFrameShadow(QFrame::Plain);

    rbClimb = new QRadioButton(frame_2);
    rbClimb->setObjectName(QStringLiteral("rbClimb"));
    rbConventional = new QRadioButton(frame_2);
    rbConventional->setObjectName(QStringLiteral("rbConventional"));

    QVBoxLayout* verticalLayout_4 = new QVBoxLayout(frame_2);
    verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
    verticalLayout_4->setContentsMargins(0, 0, 0, 0);
    verticalLayout_4->addWidget(rbClimb);
    verticalLayout_4->addWidget(rbConventional);

    dsbDepth = new QDoubleSpinBox(groupBox_2);
    dsbDepth->setObjectName(QStringLiteral("dsbDepth"));
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(dsbDepth->sizePolicy().hasHeightForWidth());
    dsbDepth->setSizePolicy(sizePolicy);
    dsbDepth->setMaximum(1000);

    comboBox = new QComboBox(groupBox_2);
    comboBox->setObjectName(QStringLiteral("comboBox"));
    comboBox->setEnabled(false);

    label_17 = new QLabel(groupBox_2);
    label_17->setObjectName(QStringLiteral("label_17"));
    label_17->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    label_2 = new QLabel(groupBox_2);
    label_2->setObjectName(QStringLiteral("label_2"));
    label_2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    lblPixmap = new QLabel(groupBox_2);
    lblPixmap->setObjectName(QStringLiteral("label_15"));
    QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(lblPixmap->sizePolicy().hasHeightForWidth());
    lblPixmap->setSizePolicy(sizePolicy1);

    label_6 = new QLabel(groupBox_2);
    label_6->setObjectName(QStringLiteral("label_6"));
    label_6->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    label_13 = new QLabel(groupBox_2);
    label_13->setObjectName(QStringLiteral("label_13"));
    label_13->setAlignment(Qt::AlignRight | Qt::AlignTop);

    QFrame* line_4 = new QFrame(groupBox_2);
    line_4->setObjectName(QStringLiteral("line_4"));
    line_4->setFrameShape(QFrame::HLine);
    line_4->setFrameShadow(QFrame::Sunken);

    pushButton_4 = new QPushButton(groupBox_2);
    pushButton_4->setObjectName(QStringLiteral("pushButton_4"));

    pushButton_3 = new QPushButton(groupBox_2);
    pushButton_3->setObjectName(QStringLiteral("pushButton_3"));

    label_16 = new QLabel(groupBox_2);
    label_16->setObjectName(QStringLiteral("label_16"));
    label_16->setEnabled(true);
    label_16->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    pushButton_6 = new QPushButton(groupBox_2);
    pushButton_6->setObjectName(QStringLiteral("pushButton_6"));
    pushButton_6->setEnabled(false);

    QFrame* line = new QFrame(groupBox_2);
    line->setObjectName(QStringLiteral("line"));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    label_12 = new QLabel(groupBox_2);
    label_12->setObjectName(QStringLiteral("label_12"));

    label_4 = new QLabel(groupBox_2);
    label_4->setObjectName(QStringLiteral("label_4"));

    label_5 = new QLabel(groupBox_2);
    label_5->setObjectName(QStringLiteral("label_5"));

    label_14 = new QLabel(groupBox_2);
    label_14->setObjectName(QStringLiteral("label_14"));
    label_14->setAlignment(Qt::AlignRight | Qt::AlignTop);

    pushButton_5 = new QPushButton(groupBox_2);
    pushButton_5->setObjectName(QStringLiteral("pushButton_5"));
    pushButton_5->setEnabled(false);

    label_3 = new QLabel(groupBox_2);
    label_3->setObjectName(QStringLiteral("label_3"));
    label_3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QFrame* line_2 = new QFrame(groupBox_2);
    line_2->setObjectName(QStringLiteral("line_2"));
    line_2->setFrameShape(QFrame::HLine);
    line_2->setFrameShadow(QFrame::Sunken);

    dsbRasterAngle = new QDoubleSpinBox(groupBox_2);
    dsbRasterAngle->setObjectName(QStringLiteral("dsbRasterAngle"));
    dsbRasterAngle->setEnabled(false);

    checkBox = new QCheckBox(groupBox_2);
    checkBox->setObjectName(QStringLiteral("checkBox"));
    checkBox->setEnabled(false);

    QGridLayout* gridLayout_2 = new QGridLayout(groupBox_2);
    gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
    gridLayout_2->setMargin(6);
    gridLayout_2->addWidget(frame_1, 10, 1, 1, 1);
    gridLayout_2->addWidget(frame_2, 11, 1, 1, 1);
    gridLayout_2->addWidget(dsbDepth, 0, 1, 1, 2);
    gridLayout_2->addWidget(comboBox, 13, 1, 1, 2);
    gridLayout_2->addWidget(label_17, 13, 0, 1, 1);
    gridLayout_2->addWidget(label_2, 0, 0, 1, 1);
    gridLayout_2->addWidget(lblPixmap, 10, 2, 2, 1);
    gridLayout_2->addWidget(label_6, 6, 0, 1, 1);
    gridLayout_2->addWidget(label_13, 10, 0, 1, 1);
    gridLayout_2->addWidget(line_4, 8, 0, 1, 3);
    gridLayout_2->addWidget(pushButton_4, 3, 2, 1, 1);
    gridLayout_2->addWidget(pushButton_3, 3, 1, 1, 1);
    gridLayout_2->addWidget(label_16, 12, 0, 1, 1);
    gridLayout_2->addWidget(pushButton_6, 7, 1, 1, 1);
    gridLayout_2->addWidget(line, 1, 0, 1, 3);
    gridLayout_2->addWidget(label_12, 9, 0, 1, 3);
    gridLayout_2->addWidget(label_4, 2, 1, 1, 2);
    gridLayout_2->addWidget(label_5, 6, 1, 1, 2);
    gridLayout_2->addWidget(label_14, 11, 0, 1, 1);
    gridLayout_2->addWidget(pushButton_5, 7, 2, 1, 1);
    gridLayout_2->addWidget(label_3, 2, 0, 1, 1);
    gridLayout_2->addWidget(line_2, 4, 0, 1, 3);
    gridLayout_2->addWidget(dsbRasterAngle, 12, 1, 1, 2);
    gridLayout_2->addWidget(checkBox, 5, 0, 1, 3);
    //groupBox_3
    groupBox_3 = new QGroupBox(Form);
    groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
    nameForm = new ToolpathNameForm(groupBox_3);
    nameForm->setObjectName(QStringLiteral("nameForm"));

    QVBoxLayout* vLayout = new QVBoxLayout(groupBox_3);
    vLayout->setObjectName(QStringLiteral("gridLayout"));
    vLayout->setMargin(6);
    vLayout->addWidget(nameForm);

    pbCalculate = new QPushButton(Form);
    pbCalculate->setObjectName(QStringLiteral("pbCalculate"));
    connect(pbCalculate, &QPushButton::clicked, this, &PocketToolpathForm::calculate);

    pbClose = new QPushButton(Form);
    pbClose->setObjectName(QStringLiteral("pbClose"));
    connect(pbClose, &QPushButton::clicked, [=]() { if (static_cast<QDockWidget*>(parent())) static_cast<QDockWidget*>(parent())->hide(); });

    QVBoxLayout* verticalLayout = new QVBoxLayout(Form);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    verticalLayout->setMargin(6);
    verticalLayout->addWidget(groupBox_1);
    verticalLayout->addWidget(groupBox_2);
    verticalLayout->addWidget(groupBox_3);
    verticalLayout->addWidget(pbCalculate);
    verticalLayout->addWidget(pbClose);
    verticalLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    rbList = (QList<QRadioButton*>() = { rbOffset, rbRaster, rbClimb, rbConventional });

    //    QString style1 = "QGroupBox {"
    //                     "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #80FF80, stop: 1 #FFFFFF);"
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

void PocketToolpathForm::retranslateUi(QWidget* Form)
{
    Form->setWindowTitle(tr("ProfileToolpathForm"));
    checkBox->setText(tr("Use Large Area Clearance Tool "));
    comboBox->clear();
    comboBox->insertItems(0, QStringList() << tr("No Profile Pass") << tr("First") << tr("Last"));
    dsbDepth->setSuffix(tr(" mm"));
    dsbRasterAngle->setSuffix(tr(" \302\260"));
    groupBox_2->setTitle(QString());
    groupBox_3->setTitle(QString());
    label->setText(tr("Pocket Toolpath"));
    label_12->setText(tr("Clear Pocket..."));
    label_13->setText(tr("Type:"));
    label_14->setText(tr("Direction:"));
    label_16->setText(tr("Raster Angle:"));
    label_17->setText(tr("Profile Pass:"));
    label_2->setText(tr("Cut Depth:"));
    label_3->setText(tr("Tool:"));
    label_4->setText(tr("TextLabel"));
    label_5->setText(tr("Not using area clear tool"));
    label_6->setText(tr("Tool:"));
    lblPixmap->setText(tr("TextLabel"));
    pbCalculate->setText(tr("Calculate"));
    pbClose->setText(tr("Close"));
    pushButton_3->setText(tr("Edit..."));
    pushButton_4->setText(tr("Select..."));
    pushButton_5->setText(tr("Select..."));
    pushButton_6->setText(tr("Edit..."));
    rbClimb->setText(tr("Climb"));
    rbConventional->setText(tr("Conventional"));
    rbOffset->setText(tr("Offser"));
    rbRaster->setText(tr("Raster"));
    groupBox_1->setTitle(QString());
}

void PocketToolpathForm::calculate()
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

    double toolDiameter = dsbDepth->value();
    ToolPathCreator tpc;
    tpc.addPaths(value);
    Pathss paths = tpc.ToolPathPocket(INSIDE_MILLING, toolDiameter);
    //tpc.Clear();

    QPainterPath painterPath;
    QList<QGraphicsItem*> items;
    QGraphicsPathItem* pathItem;

    //    ClipperOffset offset(2.0, uScale / 10000);
    //    //for (Path path : paths) {
    //    Paths paths2;
    //    //offset.Clear();
    //    //offset.AddPath(path, jtRound, etOpenRound);
    //    offset.AddPaths(paths, jtRound, etOpenRound);
    //    offset.Execute(paths2, toolDiameter * 0.5 * uScale);
    //    for (Path path : paths2) {
    //        painterPath.addPolygon(PathToQPolygon(path));
    //    }
    //    for (Path path : paths2) {
    //        painterPath.addPolygon(PathToQPolygon(path));
    //    }
    //    pathItem = new QGraphicsPathItem(painterPath);
    //    pathItem->setPen(Qt::NoPen);
    //    pathItem->setBrush(QColor(255, 255, 255, 100));
    //    items.append(pathItem);
    //    //}

    painterPath = QPainterPath();
    for (Paths &paths2 : paths) {
        for (Path &path : paths2) {
//            path.append(path.first());
            painterPath.addPolygon(PathToQPolygon(path));
        }
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
