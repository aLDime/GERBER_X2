#include "pocketform.h"
#include "materialsetupform.h"
#include "ui_pocketform.h"

#include "filetree/filemodel.h"
#include "gcode/gcode.h"
#include "gcode/toolpathcreator.h"
#include "tooldatabase/tooldatabase.h"
#include <QDockWidget>
#include <QMessageBox>
#include <QSettings>
#include <myclipper.h>
#include <scene.h>
#include <tooldatabase/tooleditdialog.h>

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
        if (ui->rbOffset->isChecked())
            type = Offset;
        else if (ui->rbRaster->isChecked())
            type = Raster;

        ui->cbxPass->setEnabled(ui->rbRaster->isChecked());
        ui->dsbxAngle->setEnabled(ui->rbRaster->isChecked());

        if (ui->rbClimb->isChecked())
            direction = Climb;
        else if (ui->rbConventional->isChecked())
            direction = Conventional;

        updateName();
        updatePixmap();
    };

    QSettings settings;
    settings.beginGroup("PocketForm");
    if (settings.value("rbClimb").toBool())
        ui->rbClimb->setChecked(true);
    if (settings.value("rbConventional").toBool())
        ui->rbConventional->setChecked(true);
    if (settings.value("rbInside").toBool())
        ui->rbInside->setChecked(true);
    if (settings.value("rbOffset").toBool())
        ui->rbOffset->setChecked(true);
    if (settings.value("rbOutside").toBool())
        ui->rbOutside->setChecked(true);
    if (settings.value("rbRaster").toBool())
        ui->rbRaster->setChecked(true);
    ui->checkBox->setChecked(settings.value("checkBox").toBool());
    settings.endGroup();

    ui->pbEdit->setIcon(QIcon::fromTheme("document-edit"));
    ui->pbSelect->setIcon(QIcon::fromTheme("tools-wizard"));
    ui->pbEdit_2->setIcon(QIcon::fromTheme("document-edit"));
    ui->pbSelect_2->setIcon(QIcon::fromTheme("tools-wizard"));
    ui->pbClose->setIcon(QIcon::fromTheme("window-close"));
    ui->pbCreate->setIcon(QIcon::fromTheme("document-export"));

    ui->sbxSteps->setSuffix(" - Infinity");

    rb_clicked();
    connect(ui->rbClimb, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbConventional, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbInside, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbOffset, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbOutside, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbRaster, &QRadioButton::clicked, rb_clicked);
}

PocketForm::~PocketForm()
{
    qDebug("~PocketForm()");
    QSettings settings;
    settings.beginGroup("PocketForm");
    settings.setValue("rbClimb", ui->rbClimb->isChecked());
    settings.setValue("rbConventional", ui->rbConventional->isChecked());
    settings.setValue("rbInside", ui->rbInside->isChecked());
    settings.setValue("rbOffset", ui->rbOffset->isChecked());
    settings.setValue("rbOutside", ui->rbOutside->isChecked());
    settings.setValue("rbRaster", ui->rbRaster->isChecked());
    settings.setValue("checkBox", ui->checkBox->isChecked());
    settings.endGroup();
    delete ui;
}

void PocketForm::on_pbSelect_clicked()
{
    ToolDatabase tdb(this, { Tool::EndMill, Tool::Engraving });
    if (tdb.exec()) {
        tool = tdb.tool();
        ui->lblToolName->setText(tool.name);
        updateName();
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

void PocketForm::on_pbEdit_clicked()
{
    ToolEditDialog d;
    d.toolEdit->setTool(tool);
    if (d.exec()) {
        tool = d.toolEdit->tool();
        ui->lblToolName->setText(tool.name);
        updateName();
    }
}

void PocketForm::on_pbEdit_2_clicked()
{
    ToolEditDialog d;
    d.toolEdit->setTool(tool2);
    if (d.exec()) {
        tool2 = d.toolEdit->tool();
        ui->lblToolName_2->setText(tool2.name);
        updateName();
    }
}

void PocketForm::on_pbCreate_clicked()
{
    create();
}

void PocketForm::on_pbClose_clicked()
{
    if (parent())
        static_cast<QWidget*>(parent())->close();
}

void PocketForm::create()
{
    Scene* scene = Scene::self;

    if (!tool.isValid()) {
        QMessageBox::warning(this, "No valid tool...!!!", tool.errorStr());
        return;
    }

    if (ui->chbxUseTwoTools->isChecked() && !tool2.isValid()) {
        QMessageBox::warning(this, "No valid tool...!!!", tool2.errorStr());
        return;
    }

    Paths wPaths;
    Side boardSide = NullSide;

    G::File const* file = nullptr;

    for (QGraphicsItem* item : scene->selectedItems()) {
        if (item->type() == GerberItemType) {
            GerberItem* gi = static_cast<GerberItem*>(item);
            //            if (boardSide == NullSide) {
            if (!file)
                file = gi->file();
            if (file != gi->file()) {
                QMessageBox::warning(this, "", "Working items from different files!");
                return;
            }
            boardSide = gi->file()->side();
            //            }
            //            if (boardSide != gi->file()->side()) {
            //                QMessageBox::warning(this, "", "Working items from different sides!");
            //                return;
            //            }
        }
        if (item->type() == GerberItemType || item->type() == DrillItemType)
            wPaths.append(static_cast<GraphicsItem*>(item)->paths());
    }

    if (boardSide == NullSide)
        boardSide = Top;

    if (wPaths.isEmpty()) {
        QMessageBox::warning(this, "!!!", tr("No selected..."));
        return;
    }

    //    if (ui->chbxUseTwoTools->isChecked()) {
    //        QVector<GCodeFile*> gcode = ToolPathCreator(wPaths).createPocket2({ tool, tool2 }, ui->rbConventional->isChecked(), ui->dsbxDepth->value(), ui->rbOutside->isChecked(), ui->sbxSteps->value());

    //    } else {
    GCodeFile* gcode = ToolPathCreator(wPaths, ui->rbConventional->isChecked()).createPocket(tool, ui->dsbxDepth->value(), ui->rbOutside->isChecked(), ui->sbxSteps->value(), ui->checkBox->isChecked());
    if (gcode == nullptr) {
        QMessageBox::information(this, "!!!", tr("The tool does not fit in the allocated region!"));
        return;
    }
    gcode->setFileName(ui->leName->text());
    gcode->setSide(boardSide);
    FileModel::self->addGcode(gcode);
    //    }
}

void PocketForm::on_sbxSteps_valueChanged(int arg1)
{
    ui->sbxSteps->setSuffix(!arg1 ? " - Infinity" : "");
}

void PocketForm::updateName()
{
    QStringList name = { "Pocket Offset", "Pocket Raster" };
    ui->leName->setText(name[type] + " (" + tool.name + ")");
}

void PocketForm::on_chbxUseTwoTools_clicked(bool checked)
{
    ui->lblToolName_2->setEnabled(checked);
    ui->pbEdit_2->setEnabled(checked);
    ui->pbSelect_2->setEnabled(checked);
}

void PocketForm::updatePixmap()
{
    static const QStringList pixmapList = {
        QStringLiteral(":/toolpath/pock_offs_climb.svg"),
        QStringLiteral(":/toolpath/pock_rast_climb.svg"),
        QStringLiteral(":/toolpath/pock_offs_conv.svg"),
        QStringLiteral(":/toolpath/pock_rast_conv.svg"),
    };
    int size = qMin(ui->lblPixmap->height(), ui->lblPixmap->width());
    ui->lblPixmap->setPixmap(QIcon(pixmapList[type + direction * 2]).pixmap(QSize(size, size)));
}

void PocketForm::resizeEvent(QResizeEvent* event)
{
    updatePixmap();
    QWidget::resizeEvent(event);
}

void PocketForm::showEvent(QShowEvent* event)
{
    updatePixmap();
    QWidget::showEvent(event);
}
