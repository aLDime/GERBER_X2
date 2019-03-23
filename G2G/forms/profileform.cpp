#include "profileform.h"
#include "materialsetupform.h"
#include "normalizeraw.h"
#include "ui_profileform.h"

#include "filetree/filemodel.h"
#include "gcode/gcode.h"
#include <QDockWidget>

#include "tooldatabase/tooldatabase.h"
#include <QMessageBox>
#include <QPicture>
#include <gi/bridgeitem.h>
#include <graphicsview.h>
#include <myclipper.h>
#include <scene.h>
#include <tooldatabase/tooleditdialog.h>

ProfileForm::ProfileForm(QWidget* parent)
    : QWidget(parent)
    , ToolPathUtil("ProfileForm")
    , ui(new Ui::ProfileForm)

{
    ui->setupUi(this);
    ui->lblToolName->setText(tool.name);
    ui->dsbxDepth->setValue(MaterialSetup::thickness);

    auto rb_clicked = [&] {
        if (ui->rbOn->isChecked())
            side = On;
        if (ui->rbOutside->isChecked())
            side = Outer;
        if (ui->rbInside->isChecked())
            side = Inner;

        updateName();

        if (ui->rbClimb->isChecked())
            direction = Climb;
        else if (ui->rbConventional->isChecked())
            direction = Conventional;

        updatePixmap();
    };

    connect(ui->rbClimb, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbConventional, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbInside, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbOn, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbOutside, &QRadioButton::clicked, rb_clicked);

    QSettings settings;
    settings.beginGroup("ProfileForm");
    if (settings.value("rbClimb").toBool())
        ui->rbClimb->setChecked(true);
    if (settings.value("rbConventional").toBool())
        ui->rbConventional->setChecked(true);
    if (settings.value("rbInside").toBool())
        ui->rbInside->setChecked(true);
    if (settings.value("rbOn").toBool())
        ui->rbOn->setChecked(true);
    if (settings.value("rbOutside").toBool())
        ui->rbOutside->setChecked(true);
    ui->dsbxBridgeLenght->setValue(settings.value("dsbxBridgeLenght").toDouble());
    settings.endGroup();

    // ui->gridLayout->addWidget(ui->labelPixmap, 0, 1, 2, 1, Qt::AlignHCenter);

    ui->pbEdit->setIcon(QIcon::fromTheme("document-edit"));
    ui->pbSelect->setIcon(QIcon::fromTheme("tools-wizard"));
    ui->pbClose->setIcon(QIcon::fromTheme("window-close"));
    ui->pbCreate->setIcon(QIcon::fromTheme("document-export"));
    ui->pbAddBridge->setIcon(QIcon::fromTheme("edit-cut"));

    rb_clicked();
}

ProfileForm::~ProfileForm()
{
    qDebug("~PocketForm()");

    QSettings settings;
    settings.beginGroup("ProfileForm");
    settings.setValue("rbClimb", ui->rbClimb->isChecked());
    settings.setValue("rbConventional", ui->rbConventional->isChecked());
    settings.setValue("rbInside", ui->rbInside->isChecked());
    settings.setValue("rbOn", ui->rbOn->isChecked());
    settings.setValue("rbOutside", ui->rbOutside->isChecked());
    settings.setValue("dsbxBridgeLenght", ui->dsbxBridgeLenght->value());
    settings.endGroup();

    for (QGraphicsItem* item : Scene::self->items()) {
        if (item->type() == BridgeType)
            delete item;
    }
    delete ui;
}

void ProfileForm::on_pbSelect_clicked()
{
    ToolDatabase tdb(this, { Tool::EndMill, Tool::Engraving });
    if (tdb.exec()) {
        tool = tdb.tool();
        ui->lblToolName->setText(tool.name);
        updateName();
    }
}

void ProfileForm::on_pbEdit_clicked()
{
    ToolEditDialog d;
    d.toolEdit->setTool(tool);
    if (d.exec()) {
        tool = d.toolEdit->tool();
        ui->lblToolName->setText(tool.name);
        updateName();
    }
}

void ProfileForm::on_pbCreate_clicked()
{
    create();
}

void ProfileForm::on_pbClose_clicked()
{
    if (parent())
        static_cast<QWidget*>(parent())->close();
}

void ProfileForm::create()
{
    Scene* scene = Scene::self;

    if (!tool.isValid()) {
        QMessageBox::warning(this, "No valid tool...!!!", tool.errorStr());
        return;
    }

    Paths wPaths;
    Paths wrPaths;
    Side boardSide = NullSide;

    G::File const* file = nullptr;

    for (QGraphicsItem* item : scene->selectedItems()) {
        if (item->type() == GerberItemType) {
            GerberItem* gi = static_cast<GerberItem*>(item);
            if (!file) {
                file = gi->file();
                boardSide = gi->file()->side();
            }
            if (file != gi->file()) {
                QMessageBox::warning(this, "", "Working items from different files!");
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
                QMessageBox::warning(this, "", "Working items from different files!");
                return;
            }
        }
        if (item->type() == GerberItemType || item->type() == DrillItemType)
            wPaths.append(static_cast<GraphicsItem*>(item)->paths());
        if (item->type() == RawItemType)
            wrPaths.append(static_cast<GraphicsItem*>(item)->paths());
    }

    if (!wrPaths.isEmpty()) {
        if (side == On)
            wPaths = wrPaths;
        else {
            wPaths = NormalizeRaw(wrPaths).paths();
        }
    }

    if (boardSide == NullSide)
        boardSide = Top;

    if (wPaths.isEmpty()) {
        QMessageBox::warning(this, "!!!", tr("No selected..."));
        return;
    }

    GCodeFile* gcode = ToolPathCreator(wPaths).createProfile(tool, ui->rbConventional->isChecked(), ui->dsbxDepth->value(), side);

    if (gcode == nullptr) {
        QMessageBox::information(this, "!!!", tr("The tool does not fit in the Working items!"));
        return;
    }

    gcode->setFileName(ui->leName->text());
    gcode->setSide(boardSide);
    FileModel::self->addGcode(gcode);
}

void ProfileForm::updateName()
{
    static const QStringList name = { "Profile Outside", "Profile Inside", "Profile On" };
    ui->leName->setText(name[side] + " (" + tool.name + ")");
    updateBridge();
}

void ProfileForm::resizeEvent(QResizeEvent* event)
{
    updatePixmap();
    QWidget::resizeEvent(event);
}

void ProfileForm::showEvent(QShowEvent* event)
{
    updatePixmap();
    QWidget::showEvent(event);
}

void ProfileForm::on_pbAddBridge_clicked()
{
    static BridgeItem* item = nullptr;
    if (item) {
        if (!item->ok())
            delete item;
    }
    item = new BridgeItem(m_lenght, m_size, item);
    GraphicsView::self->scene()->addItem(item);
}

void ProfileForm::on_dsbxBridgeLenght_valueChanged(double /*arg1*/)
{
    updateBridge();
}

void ProfileForm::on_dsbxDepth_valueChanged(double /*arg1*/)
{
    updateBridge();
}

void ProfileForm::updateBridge()
{
    m_lenght = ui->dsbxBridgeLenght->value();
    m_size = tool.getDiameter(ui->dsbxDepth->value());
    for (QGraphicsItem* item : Scene::self->items()) {
        if (item->type() == BridgeType)
            static_cast<BridgeItem*>(item)->update();
    }
}

void ProfileForm::updatePixmap()
{
    static const QStringList pixmapList{
        ":/toolpath/prof_on_climb.svg",
        ":/toolpath/prof_out_climb.svg",
        ":/toolpath/prof_in_climb.svg",
        ":/toolpath/prof_on_conv.svg",
        ":/toolpath/prof_out_conv.svg",
        ":/toolpath/prof_in_conv.svg",

    };
    int size = qMin(ui->lblPixmap->height(), ui->lblPixmap->width());
    ui->lblPixmap->setPixmap(QIcon(pixmapList[side + direction * 3]).pixmap(QSize(size, size)));
}
