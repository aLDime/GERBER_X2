#include "profilewidget.h"
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QtCore/QVariant>
#include <gcode/gcode.h>
#include <mainwindow.h>
#include <myscene.h>
#include <toolpath/toolpathcreator.h>

Tool ProfileWidget::tool;

enum {
    OUTSIDE,
    INSIDE,
    ON,
};

ProfileWidget::ProfileWidget(QWidget* parent)
    : Widget(parent)
{
    setupUi(this);
    restoreTools({ &tool }, "ProfileWidget");
    setName("Profile Toolpath");

    auto rb_clicked = [&] {
        QStringList list = {
            ":/toolpath/outside_climb.png",
            ":/toolpath/inside_climb.png",
            ":/toolpath/on_climb.png",
            ":/toolpath/outside_conventional.png",
            ":/toolpath/inside_conventional.png",
            ":/toolpath/on_conventional.png"
        };

        QStringList name = { "Profile Outside", "Profile Inside", "Profile On" };

        if (rbOutside->isChecked())
            side = OUTSIDE;
        else if (rbInside->isChecked())
            side = INSIDE;
        else if (rbOn->isChecked())
            side = ON;

        pathName->setText(name[side]);

        if (rbClimb->isChecked())
            direction = CLIMB;
        else if (rbConventional->isChecked())
            direction = CONVENTIONAL;

        lblPixmap->setPixmap(QPixmap(list[side + direction * 3]));
    };

    connect(rbClimb, &QRadioButton::clicked, rb_clicked);
    connect(rbConventional, &QRadioButton::clicked, rb_clicked);
    connect(rbInside, &QRadioButton::clicked, rb_clicked);
    connect(rbOn, &QRadioButton::clicked, rb_clicked);
    connect(rbOutside, &QRadioButton::clicked, rb_clicked);

    rb_clicked();
}

ProfileWidget::~ProfileWidget()
{
    saveTools({ &tool }, "ProfileWidget");
}

void ProfileWidget::setupUi(QWidget* Form)
{
    if (Form->objectName().isEmpty())
        Form->setObjectName(QStringLiteral("Form"));
    //    Form->resize(246, 268);
    QGridLayout* gridLayout = new QGridLayout(Form);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));
    gridLayout->setContentsMargins(0, 0, 0, 0);
    lblPixmap = new QLabel(Form);
    lblPixmap->setObjectName(QStringLiteral("lblPixmap"));
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(lblPixmap->sizePolicy().hasHeightForWidth());
    lblPixmap->setSizePolicy(sizePolicy);
    lblPixmap->setAlignment(Qt::AlignCenter);

    gridLayout->addWidget(lblPixmap, 0, 1, 2, 1);

    groupBox_2 = new QGroupBox(Form);
    groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
    verticalLayout_2 = new QVBoxLayout(groupBox_2);
    verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
    verticalLayout_2->setContentsMargins(6, 6, 6, 6);
    rbClimb = new QRadioButton(groupBox_2);
    rbClimb->setObjectName(QStringLiteral("rbClimb"));

    verticalLayout_2->addWidget(rbClimb);

    rbConventional = new QRadioButton(groupBox_2);
    rbConventional->setObjectName(QStringLiteral("rbConventional"));

    verticalLayout_2->addWidget(rbConventional);

    gridLayout->addWidget(groupBox_2, 1, 0, 1, 1);

    groupBox = new QGroupBox(Form);
    groupBox->setObjectName(QStringLiteral("groupBox"));
    verticalLayout = new QVBoxLayout(groupBox);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    verticalLayout->setContentsMargins(6, 6, 6, 6);
    rbOutside = new QRadioButton(groupBox);
    rbOutside->setObjectName(QStringLiteral("rbOutside"));

    verticalLayout->addWidget(rbOutside);

    rbInside = new QRadioButton(groupBox);
    rbInside->setObjectName(QStringLiteral("rbInside"));

    verticalLayout->addWidget(rbInside);

    rbOn = new QRadioButton(groupBox);
    rbOn->setObjectName(QStringLiteral("rbOn"));

    verticalLayout->addWidget(rbOn);

    gridLayout->addWidget(groupBox, 0, 0, 1, 1);

    //    gridLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding), 2, 0, 1, 2);

    retranslateUi(Form);

    rbOutside->setChecked(true);
    rbClimb->setChecked(true);

    QMetaObject::connectSlotsByName(Form);
}

void ProfileWidget::retranslateUi(QWidget* Form)
{
    Form->setWindowTitle(QApplication::translate("Form", "Form", Q_NULLPTR));
    lblPixmap->setText(QApplication::translate("Form", "TextLabel", Q_NULLPTR));
    groupBox_2->setTitle(QApplication::translate("Form", "Direction", Q_NULLPTR));
    rbClimb->setText(QApplication::translate("Form", "Climb", Q_NULLPTR));
    rbConventional->setText(QApplication::translate("Form", "Conventional", Q_NULLPTR));
    groupBox->setTitle(QApplication::translate("Form", "Vectors", Q_NULLPTR));
    rbOutside->setText(QApplication::translate("Form", "Outside", Q_NULLPTR));
    rbInside->setText(QApplication::translate("Form", "Inside", Q_NULLPTR));
    rbOn->setText(QApplication::translate("Form", "On", Q_NULLPTR));
}

void ProfileWidget::calculate()
{
    MyScene* scene = MyScene::self;

    if (qFuzzyIsNull(tool.diameter)) {
        QMessageBox::warning(this, "!!!", tr("No valid tool..."));
        return;
    }

    Paths wPaths;
    for (QGraphicsItem* item : scene->selectedItems()) {
        if (item->type() == WorkItemType)
            wPaths.append(static_cast<WorkItem*>(item)->getPaths());
    }

    if (wPaths.isEmpty()) {
        QMessageBox::warning(this, "!!!", tr("No selected..."));
        return;
    }

    GCodeProfile* group = ToolPathCreator().setPaths(wPaths).ToolPathProfile(static_cast<MILLING>(side), tool, rbConventional->isChecked(), depth->value());

    if (group == nullptr) {
        QMessageBox::information(this, "!!!", tr("Еhe tool does not fit in the allocated region!"));
        return;
    }

    group->addToTheScene(scene);
    FileModel::self->addMilling(pathName->text(), group);
}

Tool ProfileWidget::getTool(int n) const
{
    switch (n) {
    case 0:
    default:
        return tool;
    }
}

void ProfileWidget::setTool(int n, const Tool& value)
{
    tool = value;
    switch (n) {
    case 0:
        toolName[n]->setText(tool.name);
    }
}
