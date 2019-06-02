#include "voronoiform.h"
#include "ui_voronoiform.h"

#include "gcode/gcode.h"
#include "gcodepropertiesform.h"
#include "gi/bridgeitem.h"
#include "tooldatabase/tooldatabase.h"
#include "tooldatabase/tooleditdialog.h"
#include <QDockWidget>
#include <QMessageBox>
#include <QPicture>
#include <graphicsview.h>
#include <myclipper.h>
#include <scene.h>

VoronoiForm::VoronoiForm(QWidget* parent)
    : FormsUtil("VoronoiForm", parent)
    , ui(new Ui::VoronoiForm)
{
    ui->setupUi(this);

    ui->lblToolName->setText(tool.name);
    ui->dsbxDepth->setValue(GCodePropertiesForm::thickness);

    updateName();

    //    QSettings settings;
    //    settings.beginGroup("VoronoiForm");
    //    settings.endGroup();

    ui->pbEdit->setIcon(Icon(ButtonEditIcon));
    ui->pbSelect->setIcon(Icon(ButtonSelectIcon));
    ui->pbClose->setIcon(Icon(ButtonCloseIcon));
    ui->pbCreate->setIcon(Icon(ButtonCreateIcon));
    parent->setWindowTitle(ui->label->text());

    for (QPushButton* button : findChildren<QPushButton*>()) {
        button->setIconSize({ 16, 16 });
    }
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
    d.setTool(tool);
    if (d.exec()) {
        tool = d.tool();
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
    if (!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    Paths wPaths;
    Paths wRawPaths;
    AbstractFile const* file = nullptr;

    for (QGraphicsItem* item : Scene::selectedItems()) {
        GraphicsItem* gi = static_cast<GraphicsItem*>(item);
        switch (item->type()) {
        case GerberItemType:
            //GerberItem* gi = static_cast<GerberItem*>(item);
            if (!file) {
                file = gi->file();
                boardSide = gi->file()->side();
            }
            if (file != gi->file()) {
                QMessageBox::warning(this, "", tr("Working items from different files!"));
                return;
            }
            wPaths.append(static_cast<GraphicsItem*>(item)->paths());

            break;
        case RawItemType:
            //RawItem* gi = static_cast<RawItem*>(item);
            if (!file) {
                file = gi->file();
                boardSide = gi->file()->side();
            }
            if (file != gi->file()) {
                QMessageBox::warning(this, "", tr("Working items from different files!"));
                return;
            }
            wRawPaths.append(static_cast<GraphicsItem*>(item)->paths());

            break;
        case DrillItemType:
            //            if (static_cast<DrillItem*>(item)->isSlot())
            //                wrPaths.append(static_cast<GraphicsItem*>(item)->paths());
            //            else
            wPaths.append(static_cast<GraphicsItem*>(item)->paths());
            break;
        default:
            break;
        }
    }

    if (wPaths.isEmpty() && wRawPaths.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No selected items for working..."));
        return;
    }

    ToolPathCreator* tps = toolPathCreator(wPaths, true, side);
    tps->addRawPaths(wRawPaths);
    connect(this, &VoronoiForm::createVoronoi, tps, &ToolPathCreator::createVoronoi);
    emit createVoronoi(tool, ui->dsbxDepth->value(), ui->doubleSpinBox->value());
    showProgress();
    //    progrfess(1, 0);
}

void VoronoiForm::updateName()
{
    ui->leName->setText(tr("Voronoi"));
}

void VoronoiForm::on_leName_textChanged(const QString& arg1)
{
    m_fileName = arg1;
}


void VoronoiForm::editFile(GCodeFile *file)
{
}
