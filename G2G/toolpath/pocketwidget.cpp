#include "pocketwidget.h"
#include "toolpathcreator.h"

#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QVBoxLayout>

#include <mainwindow.h>

#include <gcode/gcode.h>
#include <graphicsview/mygraphicsscene.h>

enum {
    OFFSET,
    RASTER,
};

PocketWidget::PocketWidget(QWidget* parent)
    : Widget(parent)
{
    setupUi(this);
    restoreTool({ &tool1, &tool2 }, "PocketWidget");
    setName("Pocket Toolpath");

    rbOffset->setChecked(true);
    rbRaster->setEnabled(false);
    rbClimb->setChecked(true);

    auto rb_clicked = [&] {
        QStringList list = {
            ":/toolpath/offset_climb.png",
            ":/toolpath/raster_climb.png",
            ":/toolpath/offset_conventional.png",
            ":/toolpath/raster_conventional.png",
        };

        QStringList name = { "Pocket Offset", "Pocket Raster" };

        if (rbOffset->isChecked())
            type = OFFSET;
        else if (rbRaster->isChecked())
            type = RASTER;

        cbxProfilePass->setEnabled(rbRaster->isChecked());
        dsbxRasterAngle->setEnabled(rbRaster->isChecked());

        pathName->setText(name[type]);

        if (rbClimb->isChecked())
            direction = CLIMB;
        else if (rbConventional->isChecked())
            direction = CONVENTIONAL;

        lblPixmap->setPixmap(QPixmap(list[type + direction * 2]));
    };

    connect(rbClimb, &QRadioButton::clicked, rb_clicked);
    connect(rbConventional, &QRadioButton::clicked, rb_clicked);
    connect(rbOffset, &QRadioButton::clicked, rb_clicked);
    connect(rbRaster, &QRadioButton::clicked, rb_clicked);

    rb_clicked();
}

PocketWidget::~PocketWidget()
{
    saveTool({ &tool1, &tool2 }, "PocketWidget");
}

void PocketWidget::setupUi(QWidget* Form)
{
    if (Form->objectName().isEmpty())
        Form->setObjectName(QStringLiteral("Form"));
    Form->resize(248, 212);
    gridLayout = new QGridLayout(Form);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));
    gridLayout->setContentsMargins(0, 0, 0, 0);
    lblPixmap = new QLabel(Form);
    lblPixmap->setObjectName(QStringLiteral("lblPixmap"));

    gridLayout->addWidget(lblPixmap, 0, 1, 2, 1);

    formLayout = new QFormLayout();
    formLayout->setObjectName(QStringLiteral("formLayout"));
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    formLayout->setContentsMargins(0, 0, 0, 0);
    label_2 = new QLabel(Form);
    label_2->setObjectName(QStringLiteral("label_2"));

    formLayout->setWidget(0, QFormLayout::LabelRole, label_2);

    label_3 = new QLabel(Form);
    label_3->setObjectName(QStringLiteral("label_3"));

    formLayout->setWidget(1, QFormLayout::LabelRole, label_3);

    dsbxRasterAngle = new QDoubleSpinBox(Form);
    dsbxRasterAngle->setObjectName(QStringLiteral("dsbxRasterAngle"));
    dsbxRasterAngle->setDecimals(2);
    dsbxRasterAngle->setMaximum(180);
    dsbxRasterAngle->setSingleStep(0.5);

    formLayout->setWidget(0, QFormLayout::FieldRole, dsbxRasterAngle);

    cbxProfilePass = new QComboBox(Form);
    cbxProfilePass->setObjectName(QStringLiteral("cbxProfilePass"));

    formLayout->setWidget(1, QFormLayout::FieldRole, cbxProfilePass);

    gridLayout->addLayout(formLayout, 2, 0, 1, 2);

    groupBox = new QGroupBox(Form);
    groupBox->setObjectName(QStringLiteral("groupBox"));
    verticalLayout = new QVBoxLayout(groupBox);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    verticalLayout->setContentsMargins(6, 6, 6, 6);
    rbOffset = new QRadioButton(groupBox);
    rbOffset->setObjectName(QStringLiteral("rbOffset"));

    verticalLayout->addWidget(rbOffset);

    rbRaster = new QRadioButton(groupBox);
    rbRaster->setObjectName(QStringLiteral("rbRaster"));

    verticalLayout->addWidget(rbRaster);

    gridLayout->addWidget(groupBox, 0, 0, 1, 1);

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

    retranslateUi(Form);

    QMetaObject::connectSlotsByName(Form);
}

void PocketWidget::retranslateUi(QWidget* Form)
{
    cbxProfilePass->insertItems(0, { tr("No Profile Pass"), tr("First"), tr("Last") });
    dsbxRasterAngle->setSuffix(QApplication::translate("Form", " \302\260", Q_NULLPTR));
    Form->setWindowTitle(QApplication::translate("Form", "Form", Q_NULLPTR));
    groupBox->setTitle(QApplication::translate("Form", "Type", Q_NULLPTR));
    groupBox_2->setTitle(QApplication::translate("Form", "Direction", Q_NULLPTR));
    label_2->setText(QApplication::translate("Form", "Raster Angle:", Q_NULLPTR));
    label_3->setText(QApplication::translate("Form", "Profile Pass:", Q_NULLPTR));
    lblPixmap->setText(QApplication::translate("Form", "Pixmap", Q_NULLPTR));
    rbClimb->setText(QApplication::translate("Form", "Climb", Q_NULLPTR));
    rbConventional->setText(QApplication::translate("Form", "Conventional", Q_NULLPTR));
    rbOffset->setText(QApplication::translate("Form", "Offset", Q_NULLPTR));
    rbRaster->setText(QApplication::translate("Form", "Raster", Q_NULLPTR));
}

void PocketWidget::calculate()
{
    MyGraphicsScene* scene = MainWindow::getMainWindow()->getScene();

    if (qFuzzyIsNull(tool1.data.params[Diameter])) {
        QMessageBox::warning(this, "!!!", tr("No valid tool 1..."));
        return;
    }

    if (cbxTool2->isChecked() && qFuzzyIsNull(tool2.data.params[Diameter])) {
        QMessageBox::warning(this, "!!!", tr("No valid tool 2..."));
        return;
    }

    Paths wPaths;
    for (QGraphicsItem* item : scene->selectedItems()) {
        if (item->type() == G::WorkItemType)
            wPaths.append(static_cast<G::WorkItem*>(item)->getPaths());
    }
    if (wPaths.isEmpty()) {
        QMessageBox::warning(this, "!!!", tr("No selected..."));
        return;
    }

    GCodeProfile* group = ToolPathCreator().setPaths(wPaths).ToolPathPocket({ tool1 }, rbConventional->isChecked(), depth->value());

    if (group == nullptr) {
        QMessageBox::information(this, "!!!", tr("Еhe tool does not fit in the allocated region!"));
        return;
    }

    scene->addItem(group);
    Model::model->addMilling(pathName->text(), group);
}

Tool PocketWidget::getTool(int n) const
{
    switch (n) {
    case 0:
        return tool1;
    default:
        return tool2;
    }
}

void PocketWidget::setTool(int n, const Tool& value)
{

    switch (n) {
    case 0:
        tool1 = value;
        toolName[0]->setText(tool1.name);
        return;
    default:
        tool2 = value;
        toolName[1]->setText(tool2.name);
        return;
    }
}
