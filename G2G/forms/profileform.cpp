#include "profileform.h"
#include "ui_profileform.h"

#include "filetree/filemodel.h"
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

ProfileForm::ProfileForm(QWidget* parent)
    : ToolPathUtil("ProfileForm", parent)
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

        if (ui->rbClimb->isChecked())
            direction = Climb;
        else if (ui->rbConventional->isChecked())
            direction = Conventional;

        updateName();
        updatePixmap();
    };

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

    ui->pbEdit->setIcon(Icon(PuttonEditIcon));
    ui->pbSelect->setIcon(Icon(PuttonSelectIcon));
    ui->pbClose->setIcon(Icon(PuttonCloseIcon));
    ui->pbCreate->setIcon(Icon(PuttonCreateIcon));
    ui->pbAddBridge->setIcon(Icon(PuttonAddBridgeIcon));

    rb_clicked();
    connect(ui->rbClimb, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbConventional, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbInside, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbOn, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbOutside, &QRadioButton::clicked, rb_clicked);
}

ProfileForm::~ProfileForm()
{

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
        tool.id = -1;
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
        tool.errorMessageBox(this);
        return;
    }

    Paths wPaths;
    Paths wrPaths;
    Side boardSide = Top;

    Gerber::File const* file = nullptr;

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
        if (item->type() == GerberItemType)
            wPaths.append(static_cast<GraphicsItem*>(item)->paths());
        if (item->type() == DrillItemType) {
            if (static_cast<DrillItem*>(item)->isSlot())
                wrPaths.append(static_cast<GraphicsItem*>(item)->paths());
            else
                wPaths.append(static_cast<GraphicsItem*>(item)->paths());
        }
        if (item->type() == RawItemType)
            wrPaths.append(static_cast<GraphicsItem*>(item)->paths());
    }

    if (wrPaths.isEmpty() && wPaths.isEmpty()) {
        QMessageBox::warning(this, "Warning", tr("No selected items for working..."));
        return;
    }

    ToolPathCreator* tpc = toolPathCreator(wPaths, ui->rbConventional->isChecked(), side);
    tpc->addRawPaths(wrPaths);

    connect(this, &ProfileForm::createProfile, tpc, &ToolPathCreator::createProfile);
    emit createProfile(tool, ui->dsbxDepth->value());
}

void ProfileForm::updateName()
{
    static const QStringList name = { "Profile On", "Profile Outside", "Profile Inside" };
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

void ProfileForm::on_dsbxBridgeLenght_valueChanged(double /*arg1*/) { updateBridge(); }

void ProfileForm::on_dsbxDepth_valueChanged(double /*arg1*/) { updateBridge(); }

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
        QStringLiteral(":/toolpath/prof_on_climb.svg"),
        QStringLiteral(":/toolpath/prof_out_climb.svg"),
        QStringLiteral(":/toolpath/prof_in_climb.svg"),
        QStringLiteral(":/toolpath/prof_on_conv.svg"),
        QStringLiteral(":/toolpath/prof_out_conv.svg"),
        QStringLiteral(":/toolpath/prof_in_conv.svg"),
    };
    int size = qMin(ui->lblPixmap->height(), ui->lblPixmap->width());
    ui->lblPixmap->setPixmap(QIcon(pixmapList[side + direction * 3]).pixmap(QSize(size, size)));
}

void ProfileForm::on_leName_textChanged(const QString& arg1) { m_fileName = arg1; }
