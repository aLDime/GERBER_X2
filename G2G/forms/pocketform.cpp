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
    : ToolPathUtil("PocketForm", parent)
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
    on_chbxUseTwoTools_clicked(settings.value("chbxUseTwoTools").toBool());
    settings.endGroup();

    ui->pbEdit->setIcon(Icon(PuttonEditIcon));
    ui->pbSelect->setIcon(Icon(PuttonSelectIcon));
    ui->pbEdit_2->setIcon(Icon(PuttonEditIcon));
    ui->pbSelect_2->setIcon(Icon(PuttonSelectIcon));
    ui->pbClose->setIcon(Icon(PuttonCloseIcon));
    ui->pbCreate->setIcon(Icon(PuttonCreateIcon));

    ui->sbxSteps->setSuffix(tr(" - Infinity"));

    rb_clicked();
    connect(ui->rbClimb, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbConventional, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbInside, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbOffset, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbOutside, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbRaster, &QRadioButton::clicked, rb_clicked);
    parent->setWindowTitle(ui->label->text());
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
    settings.setValue("chbxUseTwoTools", ui->chbxUseTwoTools->isChecked());
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
        tool.id = -1;
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
        tool2.id = -1;
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
        tool.errorMessageBox(this);
        return;
    }

    if (ui->chbxUseTwoTools->isChecked() && !tool2.isValid()) {
        tool2.errorMessageBox(this);
        return;
    }

    Paths wPaths;
    Side boardSide = NullSide;

    Gerber::File const* file = nullptr;

    for (QGraphicsItem* item : scene->selectedItems()) {
        if (item->type() == GerberItemType) {
            GerberItem* gi = static_cast<GerberItem*>(item);
            //            if (boardSide == NullSide) {
            if (!file)
                file = gi->file();
            if (file != gi->file()) {
                QMessageBox::warning(this, "", tr("Working items from different files!"));
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
        QMessageBox::warning(this, tr("Warning"), tr("No selected items for working..."));
        return;
    }

    ToolPathCreator* tps = toolPathCreator(wPaths, ui->rbConventional->isChecked(), ui->rbOutside->isChecked() ? Outer : Inner);
    if (ui->chbxUseTwoTools->isChecked()) {
        //        QPair<GCodeFile*, GCodeFile*> files = tpc.createPocket2({ tool2, tool }, ui->dsbxDepth->value());

        //        if (files.first) {
        //            files.first->setFileName(ui->leName->text() + "1");
        //            files.first->setSide(boardSide);
        //            FileModel::addFile(files.first);
        //        } else {
        //            //QMessageBox::information(this, "Warning", tr("The tool does not fit in the allocated region!"));
        //        }

        //        if (files.second) {
        //            files.second->setFileName(ui->leName->text() + "2");
        //            files.second->setSide(boardSide);
        //            FileModel::addFile(files.second);
        //        } else {
        //            //QMessageBox::information(this, "Warning", tr("The tool does not fit in the allocated region!"));
        //        }

    } else {
        connect(this, &PocketForm::createPocket, tps, &ToolPathCreator::createPocket);
        emit createPocket(tool, ui->dsbxDepth->value(), ui->sbxSteps->value());
    }
}

void PocketForm::on_sbxSteps_valueChanged(int arg1)
{
    ui->sbxSteps->setSuffix(!arg1 ? tr(" - Infinity") : "");
}

void PocketForm::updateName()
{
    QStringList name = { tr("Pocket Offset"), tr("Pocket Raster") };
    ui->leName->setText(name[type] + " (" + tool.name + ")");
}

void PocketForm::on_chbxUseTwoTools_clicked(bool checked)
{
    ui->chbxUseTwoTools->setChecked(checked);
    ui->lblToolName_2->setEnabled(checked);
    ui->pbEdit_2->setEnabled(checked);
    ui->pbSelect_2->setEnabled(checked);
    ui->sbxSteps->setEnabled(!checked);
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

void PocketForm::on_leName_textChanged(const QString& arg1)
{
    m_fileName = arg1;
}
