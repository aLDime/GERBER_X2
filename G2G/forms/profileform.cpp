#include "profileform.h"
#include "materialsetupform.h"
#include "ui_profileform.h"

#include "filetree/filemodel.h"
#include "gcode/gcode.h"
#include <QDockWidget>

#include "gcode/toolpathcreator.h"
#include "tooldatabase/tooldatabase.h"
#include <QMessageBox>
#include <graphicsitem.h>
#include <myclipper.h>
#include <myscene.h>

enum {
    OUTSIDE,
    INSIDE,
    ON,
};

enum {
    CLIMB,
    CONVENTIONAL
};

ProfileForm::ProfileForm(QWidget* parent)
    : QWidget(parent)
    , ToolPathUtil("ProfileForm")
    , ui(new Ui::ProfileForm)
{
    ui->setupUi(this);
    ui->lblToolName->setText(tool.name);
    ui->dsbxDepth->setValue(MaterialSetupForm::thickness);

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

        if (ui->rbOutside->isChecked())
            side = OUTSIDE;
        else if (ui->rbInside->isChecked())
            side = INSIDE;
        else if (ui->rbOn->isChecked())
            side = ON;

        ui->leName->setText(name[side]);

        if (ui->rbClimb->isChecked())
            direction = CLIMB;
        else if (ui->rbConventional->isChecked())
            direction = CONVENTIONAL;

        ui->lblPixmap->setPixmap(QPixmap(list[side + direction * 3]));
    };

    connect(ui->rbClimb, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbConventional, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbInside, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbOn, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbOutside, &QRadioButton::clicked, rb_clicked);

    rb_clicked();
}

ProfileForm::~ProfileForm()
{
    delete ui;
}

void ProfileForm::on_pbSelect_clicked()
{
    ToolDatabase tdb(this, { Tool::EndMill, Tool::Engraving });
    if (tdb.exec()) {
        tool = tdb.tool();
        ui->lblToolName->setText(tool.name);
    }
}

void ProfileForm::on_pbEdit_clicked()
{
}

void ProfileForm::on_pbCreate_clicked()
{
    MyScene* scene = MyScene::self;

    if (!tool.isValid()) {
        QMessageBox::warning(this, "No valid tool...!!!", tool.errorStr());
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

    GCode* gcode = ToolPathCreator(wPaths).ToolPathProfile(static_cast<MILLING>(side), tool, ui->rbConventional->isChecked(), ui->dsbxDepth->value());

    if (gcode == nullptr) {
        QMessageBox::information(this, "!!!", tr("Еhe tool does not fit in the allocated region!"));
        return;
    }

    gcode->setName(ui->leName->text());
    FileModel::self->addGcode(gcode);
}

void ProfileForm::on_pbClose_clicked()
{
    if (parent())
        static_cast<QDockWidget*>(parent())->hide();
}
