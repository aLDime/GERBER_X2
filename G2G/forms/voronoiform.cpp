#include "voronoiform.h"
#include "ui_voronoiform.h"

#include "gcode/gcode.h"
#include "gi/bridgeitem.h"
#include "materialsetupform.h"
#include "tooldatabase/tooldatabase.h"
#include "tooldatabase/tooleditdialog.h"
#include <QDockWidget>
#include <QMessageBox>
#include <QPicture>
#include <graphicsview.h>
#include <myclipper.h>
#include <scene.h>

VoronoiForm::VoronoiForm(QWidget* parent)
    : ToolPathUtil("VoronoiForm", parent)
    , ui(new Ui::VoronoiForm)
{
    ui->setupUi(this);

    ui->lblToolName->setText(tool.name);
    ui->dsbxDepth->setValue(MaterialSetup::thickness);

    updateName();

    //    QSettings settings;
    //    settings.beginGroup("VoronoiForm");
    //    settings.endGroup();

    ui->pbEdit->setIcon(Icon(PuttonEditIcon));
    ui->pbSelect->setIcon(Icon(PuttonSelectIcon));
    ui->pbClose->setIcon(Icon(PuttonCloseIcon));
    ui->pbCreate->setIcon(Icon(PuttonCreateIcon));
    parent->setWindowTitle(ui->label->text());
}

VoronoiForm::~VoronoiForm()
{
    //    QSettings settings;
    //    settings.beginGroup("VoronoiForm");
    //    settings.endGroup();
    delete ui;
}

void VoronoiForm::on_pbSelect_clicked()
{
    ToolDatabase tdb(this, { Tool::EndMill, Tool::Engraving });
    if (tdb.exec()) {
        tool = tdb.tool();
        ui->lblToolName->setText(tool.name);
        updateName();
    }
}

void VoronoiForm::on_pbEdit_clicked()
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

void VoronoiForm::on_pbCreate_clicked()
{
    create();
}

void VoronoiForm::on_pbClose_clicked()
{
    static_cast<QWidget*>(parent())->close();
}

void VoronoiForm::create()
{
    Scene* scene = Scene::self;

    if (!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    Paths wPaths;
    Paths wRawPaths;

    Gerber::File const* file = nullptr;

    for (QGraphicsItem* item : scene->selectedItems()) {
        if (item->type() == GerberItemType) {
            GerberItem* gi = static_cast<GerberItem*>(item);
            if (!file) {
                file = gi->file();
                boardSide = gi->file()->side();
            }
            if (file != gi->file()) {
                QMessageBox::warning(this, "", tr("Working items from different files!"));
                return;
            }
        }
        if (item->type() == RawItemType) {
            RawItem* gi = static_cast<RawItem*>(item);
            if (!file) {
                file = gi->file();
                boardSide = gi->file()->side();
            }
            if (file != gi->file()) {
                QMessageBox::warning(this, "", tr("Working items from different files!"));
                return;
            }
        }
        if (item->type() == GerberItemType)
            wPaths.append(static_cast<GraphicsItem*>(item)->paths());
        if (item->type() == DrillItemType) {
            //            if (static_cast<DrillItem*>(item)->isSlot())
            //                wrPaths.append(static_cast<GraphicsItem*>(item)->paths());
            //            else
            wPaths.append(static_cast<GraphicsItem*>(item)->paths());
        }
        if (item->type() == RawItemType)
            wRawPaths.append(static_cast<GraphicsItem*>(item)->paths());
    }

    if (wPaths.isEmpty() && wRawPaths.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No selected items for working..."));
        return;
    }

    ToolPathCreator* tps = toolPathCreator(wPaths, true, side);
    tps->addRawPaths(wRawPaths);
    connect(this, &VoronoiForm::createVoronoi, tps, &ToolPathCreator::createVoronoi);
    emit createVoronoi(tool, ui->dsbxDepth->value(), ui->doubleSpinBox->value());
    progress(1, 0);
}

void VoronoiForm::updateName()
{
    ui->leName->setText(tr("Voronoi (") + tool.name + ")");
}

void VoronoiForm::on_leName_textChanged(const QString& arg1)
{
    m_fileName = arg1;
}
