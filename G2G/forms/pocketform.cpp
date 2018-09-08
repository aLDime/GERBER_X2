#include "pocketform.h"
#include "ui_pocketform.h"

#include "filetree/filemodel.h"
#include "gcode/gcode.h"
#include "gcode/toolpathcreator.h"
#include "tooldatabase/tooldatabase.h"
#include <QDockWidget>
#include <QMessageBox>
#include <graphicsitem.h>
#include <myclipper.h>
#include <myscene.h>

enum {
    OFFSET,
    RASTER,
};

enum {
    CLIMB,
    CONVENTIONAL
};

PocketForm::PocketForm(QWidget* parent)
    : QWidget(parent)
    , ToolPathUtil("PocketForm")
    , ui(new Ui::PocketForm)
{
    ui->setupUi(this);
    ui->lblToolName->setText(tool.name);
    ui->lblToolName_2->setText(tool2.name);

    auto rb_clicked = [&] {
        QStringList list = {
            ":/toolpath/offset_climb.png",
            ":/toolpath/raster_climb.png",
            ":/toolpath/offset_conventional.png",
            ":/toolpath/raster_conventional.png",
        };

        QStringList name = { "Pocket Offset", "Pocket Raster" };

        if (ui->rbOffset->isChecked())
            type = OFFSET;
        else if (ui->rbRaster->isChecked())
            type = RASTER;

        ui->cbxPass->setEnabled(ui->rbRaster->isChecked());
        ui->dsbxAngle->setEnabled(ui->rbRaster->isChecked());

        ui->leName->setText(name[type]);

        if (ui->rbClimb->isChecked())
            direction = CLIMB;
        else if (ui->rbConventional->isChecked())
            direction = CONVENTIONAL;

        ui->lblPixmap->setPixmap(QPixmap(list[type + direction * 2]));
    };

    connect(ui->rbClimb, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbConventional, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbOffset, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbRaster, &QRadioButton::clicked, rb_clicked);

    rb_clicked();
}

PocketForm::~PocketForm()
{
    delete ui;
}

void PocketForm::on_pbSelect_clicked()
{
    ToolDatabase tdb(this, { Tool::EndMill, Tool::Engraving });
    if (tdb.exec()) {
        tool = tdb.tool();
        ui->lblToolName->setText(tool.name);
    }
}
void PocketForm::on_pbSelect_2_clicked()
{
    ToolDatabase tdb(this, { Tool::EndMill, Tool::Engraving });
    if (tdb.exec()) {
        tool2 = tdb.tool();
        ui->lblToolName_2->setText(tool2.name);
    }
}

void PocketForm::on_pbEdit_clicked() {}

void PocketForm::on_pbEdit_2_clicked() {}

void PocketForm::on_pbCreate_clicked()
{
    MyScene* scene = MyScene::self;

    if (qFuzzyIsNull(tool.diameter)) {
        QMessageBox::warning(this, "!!!", tr("No valid tool 1..."));
        return;
    }

    if (ui->chbxUseTwoTools->isChecked() && qFuzzyIsNull(tool2.diameter)) {
        QMessageBox::warning(this, "!!!", tr("No valid tool 2..."));
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

    GCode* gcode = ToolPathCreator(wPaths).ToolPathPocket({ tool }, ui->rbConventional->isChecked(), ui->dsbxDepth->value());

    if (gcode == nullptr) {
        QMessageBox::information(this, "!!!", tr("Еhe tool does not fit in the allocated region!"));
        return;
    }
    gcode->addToTheScene(scene);
    gcode->setName(ui->leName->text());
    FileModel::self->addGcode(gcode);
}

void PocketForm::on_pbClose_clicked()
{
    if (parent())
        static_cast<QDockWidget*>(parent())->hide();
}
