#include "pocketform.h"
#include "materialsetupform.h"
#include "ui_pocketform.h"

#include "filetree/filemodel.h"
#include "gcode/gcode.h"
#include "gcode/toolpathcreator.h"
#include "tooldatabase/tooldatabase.h"
#include <QDockWidget>
#include <QMessageBox>
#include <myclipper.h>
#include <myscene.h>

enum {
    Offset,
    Raster,
};

PocketForm::PocketForm(QWidget* parent)
    : QWidget(parent)
    , ToolPathUtil("PocketForm")
    , ui(new Ui::PocketForm)
{
    ui->setupUi(this);
    ui->lblToolName->setText(tool.name);
    ui->lblToolName_2->setText(tool2.name);

    ui->dsbxDepth->setValue(MaterialSetup::thickness);

    auto rb_clicked = [&] {
        QStringList list = {
            ":/toolpath/offset_climb.png",
            ":/toolpath/raster_climb.png",
            ":/toolpath/offset_conventional.png",
            ":/toolpath/raster_conventional.png",
        };

        QStringList name = { "Pocket Offset", "Pocket Raster" };

        if (ui->rbOffset->isChecked())
            type = Offset;
        else if (ui->rbRaster->isChecked())
            type = Raster;

        ui->cbxPass->setEnabled(ui->rbRaster->isChecked());
        ui->dsbxAngle->setEnabled(ui->rbRaster->isChecked());

        ui->leName->setText(name[type]);

        if (ui->rbClimb->isChecked())
            direction = Climb;
        else if (ui->rbConventional->isChecked())
            direction = Conventional;

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
    create();
}

void PocketForm::on_pbClose_clicked()
{
    if (parent())
        static_cast<QDockWidget*>(parent())->hide();
}

void PocketForm::create()
{
    MyScene* scene = MyScene::self;

    if (!tool.isValid()) {
        QMessageBox::warning(this, "No valid tool...!!!", tool.errorStr());
        return;
    }

    if (ui->chbxUseTwoTools->isChecked() && !tool2.isValid()) {
        QMessageBox::warning(this, "No valid tool...!!!", tool2.errorStr());
        return;
    }

    Paths wPaths;
    G::Side boardSide = G::Side(-1);
    for (QGraphicsItem* item : scene->selectedItems()) {
        if (item->type() == GERBER_ITEM) {
            GerberItem* gi = static_cast<GerberItem*>(item);
            if (boardSide == G::Side(-1))
                boardSide = gi->file()->side;
            if (boardSide != gi->file()->side) {
                QMessageBox::warning(this, "", "Working items from different sides!");
                return;
            }
        }
        if (item->type() == GERBER_ITEM || item->type() == DRILL_ITEM)
            wPaths.append(static_cast<GraphicsItem*>(item)->paths());
    }

    if (boardSide == G::Side(-1))
        boardSide = G::Top;

    if (wPaths.isEmpty()) {
        QMessageBox::warning(this, "!!!", tr("No selected..."));
        return;
    }

    GCode* gcode = ToolPathCreator(wPaths).createPocket({ tool }, ui->rbConventional->isChecked(), ui->dsbxDepth->value(), ui->rbOutside->isChecked());

    if (gcode == nullptr) {
        QMessageBox::information(this, "!!!", tr("Еhe tool does not fit in the allocated region!"));
        return;
    }
    gcode->setFileName(ui->leName->text());
    gcode->setSide(boardSide);
    FileModel::self->addGcode(gcode);
}
