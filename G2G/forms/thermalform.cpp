#include "thermalform.h"
#include "ui_thermalform.h"

#include "filetree/fileholder.h"
#include "gcode/gcode.h"
#include "gi/bridgeitem.h"
#include "materialsetupform.h"
#include "thermalmodel.h"
#include "thermalpreviewitem.h"
#include "tooldatabase/tooldatabase.h"
#include "tooldatabase/tooleditdialog.h"
#include <QCheckBox>
#include <QDockWidget>
#include <QMessageBox>
#include <QPicture>
#include <QTimer>
#include <graphicsview.h>
#include <myclipper.h>
#include <scene.h>

enum { Size = 24 };

extern QIcon drawApertureIcon(Gerber::AbstractAperture* aperture);

QIcon drawRegionIcon(const Gerber::GraphicObject& go)
{
    QPainterPath painterPath;

    for (QPolygonF& polygon : toQPolygons(go.paths))
        painterPath.addPolygon(polygon);

    const QRectF rect = painterPath.boundingRect();

    qreal scale = (double)Size / qMax(rect.width(), rect.height());

    double ky = rect.bottom() * scale;
    double kx = rect.left() * scale;
    if (rect.width() > rect.height())
        ky += (Size - rect.height() * scale) / 2;
    else
        kx -= (Size - rect.width() * scale) / 2;

    QPixmap pixmap(Size, Size);
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    //    painter.translate(tr);
    painter.translate(-kx, ky);
    painter.scale(scale, -scale);
    painter.drawPath(painterPath);
    return QIcon(pixmap);
}

ThermalForm::ThermalForm(QWidget* parent)
    : ToolPathUtil("ThermalForm", parent)
    , ui(new Ui::ThermalForm)
{
    ui->setupUi(this);

    ui->treeView->setIconSize(QSize(Size, Size));

    ui->lblToolName->setText(tool.name);
    ui->dsbxDepth->setValue(MaterialSetup::thickness);

    updateName();

    //    QSettings settings;
    //    settings.beginGroup("ProfileForm");
    //    settings.endGroup();

    lay = new QGridLayout(ui->treeView->header());
    cbx = new QCheckBox("", ui->treeView);
    lay->addWidget(cbx, 0, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    lay->setContentsMargins(3, 0, 0, 0);
    cbx->setMinimumHeight(ui->treeView->header()->height() - 4);
    connect(cbx, &QCheckBox::toggled, [=](bool /*checked*/) {
        //        model->setCreate(checked);
        //        updateCreateButton();
    });

    updateFiles();

    ui->pbEdit->setIcon(Icon(PuttonEditIcon));
    ui->pbSelect->setIcon(Icon(PuttonSelectIcon));
    ui->pbClose->setIcon(Icon(PuttonCloseIcon));
    ui->pbCreate->setIcon(Icon(PuttonCreateIcon));
    parent->setWindowTitle(ui->label->text());
    //    connect(ui->pbClose, &QPushButton::clicked, parent, &QWidget::close);
}

ThermalForm::~ThermalForm()
{
    m_sourcePreview.clear();
    //    QSettings settings;
    //    settings.beginGroup("ProfileForm");
    //    settings.endGroup();
    delete ui;
}

void ThermalForm::updateFiles()
{
    disconnect(ui->cbxFile, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ThermalForm::on_cbxFileCurrentIndexChanged);

    ui->cbxFile->clear();

    for (Gerber::File* file : FileHolder::files<Gerber::File>()) {
        if (file->flashedApertures()) {
            ui->cbxFile->addItem(file->shortFileName(), QVariant::fromValue(static_cast<void*>(file)));
            QPixmap pixmap(Size, Size);
            QColor color(file->color());
            color.setAlpha(255);
            pixmap.fill(color);
            ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QIcon(pixmap), Qt::DecorationRole);
            ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QSize(0, Size), Qt::SizeHintRole);
        }
    }

    if (!ui->cbxFile->count()) {
        QMessageBox::information(this, "", tr("No data to process."));
        QTimer::singleShot(1, Qt::CoarseTimer, [=] { on_pbClose_clicked(); });
    } else
        on_cbxFileCurrentIndexChanged(0);

    connect(ui->cbxFile, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ThermalForm::on_cbxFileCurrentIndexChanged);
}

void ThermalForm::on_pbSelect_clicked()
{
    ToolDatabase tdb(this, { Tool::EndMill, Tool::Engraving });
    if (tdb.exec()) {
        tool = tdb.tool();
        ui->lblToolName->setText(tool.name);
        updateName();
        redraw();
    }
}

void ThermalForm::on_pbEdit_clicked()
{
    ToolEditDialog d;
    d.toolEdit->setTool(tool);
    if (d.exec()) {
        tool = d.toolEdit->tool();
        tool.id = -1;
        ui->lblToolName->setText(tool.name);
        updateName();
        redraw();
    }
}

void ThermalForm::on_pbCreate_clicked()
{
    create();
}

void ThermalForm::on_pbClose_clicked()
{
    static_cast<QWidget*>(parent())->close();
}

void ThermalForm::on_leName_textChanged(const QString& arg1)
{
    m_fileName = arg1;
}

void ThermalForm::create()
{
    //    Scene* scene = Scene::self;

    if (!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    Paths wPaths;
    Pathss wBridgePaths;

    for (QSharedPointer<ThermalPreviewItem> item : m_sourcePreview) {
        if (item->flags() & QGraphicsItem::ItemIsSelectable) {
            wPaths.append(item->paths());
            wBridgePaths.append(item->bridge());
        }
    }

    //    for (QGraphicsItem* item : scene->selectedItems()) {
    //        if (item->type() == GerberItemType) {
    //            GerberItem* gi = static_cast<GerberItem*>(item);
    //            if (!file) {
    //                file = gi->file();
    //                boardSide = gi->file()->side();
    //            }
    //            if (file != gi->file()) {
    //                QMessageBox::warning(this, "", tr("Working items from different files!"));
    //                return;
    //            }
    //        }
    //        if (item->type() == RawItemType) {
    //            RawItem* gi = static_cast<RawItem*>(item);
    //            if (!file) {
    //                file = gi->file();
    //                boardSide = gi->file()->side();
    //            }
    //            if (file != gi->file()) {
    //                QMessageBox::warning(this, "", tr("Working items from different files!"));
    //                return;
    //            }
    //        }
    //        if (item->type() == GerberItemType)
    //            wPaths.append(static_cast<GraphicsItem*>(item)->paths());
    //        if (item->type() == DrillItemType) {
    //            //            if (static_cast<DrillItem*>(item)->isSlot())
    //            //                wrPaths.append(static_cast<GraphicsItem*>(item)->paths());
    //            //            else
    //            wPaths.append(static_cast<GraphicsItem*>(item)->paths());
    //        }
    //        if (item->type() == RawItemType)
    //            wRawPaths.append(static_cast<GraphicsItem*>(item)->paths());
    //    }

    //    if (wPaths.isEmpty() && wRawPaths.isEmpty()) {
    //        QMessageBox::warning(this, tr("Warning"), tr("No selected items for working..."));
    //        return;
    //    }

    ToolPathCreator* tps = toolPathCreator(wPaths, true, side);
    tps->addSupportPaths(wBridgePaths);
    connect(this, &ThermalForm::createThermal, tps, &ToolPathCreator::createThermal);
    emit createThermal(static_cast<Gerber::File*>(ui->cbxFile->currentData().value<void*>()), tool, ui->dsbxDepth->value());
    progress(1, 0);
}

void ThermalForm::updateName()
{
    ui->leName->setText(tr("Thermal (") + tool.name + ")");
}

void ThermalForm::on_cbxFileCurrentIndexChanged(int /*index*/)
{
    setApertures(static_cast<Gerber::File*>(ui->cbxFile->currentData().value<void*>())->apertures());
}

void ThermalForm::setApertures(const QMap<int, QSharedPointer<Gerber::AbstractAperture>>* value)
{
    m_sourcePreview.clear();

    m_apertures = *value;
    model = new ThermalModel(this);
    const Gerber::File* file = static_cast<Gerber::File*>(ui->cbxFile->currentData().value<void*>());

    QMap<int, QSharedPointer<Gerber::AbstractAperture>>::const_iterator apertureIt;
    for (apertureIt = m_apertures.begin(); apertureIt != m_apertures.end(); ++apertureIt) {
        if (apertureIt.value()->isFlashed()) {
            QString name(apertureIt.value()->name());
            ThermalNode* thermalNode = model->appendRow(drawApertureIcon(apertureIt.value().data()), name);
            for (const Gerber::GraphicObject& go : *file) {
                if (go.state.dCode() == Gerber::D03 && go.state.aperture() == apertureIt.key()) {
                    ThermalPreviewItem* item = new ThermalPreviewItem(go, tool, m_depth);
                    m_sourcePreview.append(QSharedPointer<ThermalPreviewItem>(item));
                    Scene::self->addItem(item);
                    thermalNode->append(new ThermalNode(drawRegionIcon(go), name, 0.0, 0.5, 4, go.state.curPos(), item));
                }
            }
        }
    }

    ThermalNode* thermalNode = model->appendRow(QIcon(), "Region");
    for (const Gerber::GraphicObject& go : *file) {
        if (go.state.type() == Gerber::Region && go.state.imgPolarity() == Gerber::Positive) {
            ThermalPreviewItem* item = new ThermalPreviewItem(go, tool, m_depth);
            m_sourcePreview.append(QSharedPointer<ThermalPreviewItem>(item));
            Scene::self->addItem(item);
            thermalNode->append(new ThermalNode(drawRegionIcon(go), "Region", 0.0, 0.5, 4, go.state.curPos(), item));
        }
    }
    //    updateCreateButton();

    delete ui->treeView->model();
    ui->treeView->setModel(model);
    ui->treeView->header()->setSectionResizeMode(QHeaderView::Stretch);
    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->treeView->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ThermalForm::on_selectionChanged);
}

void ThermalForm::on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    for (QModelIndex index : selected.indexes()) {
        ThermalNode* node = reinterpret_cast<ThermalNode*>(index.internalPointer());
        ThermalPreviewItem* item = node->item();
        if (item)
            item->setSelected(true);
        else {
            for (int i = 0; i < node->childCount(); ++i) {
                ui->treeView->selectionModel()->select(model->createIndex(i, 0, node->child(i)), QItemSelectionModel::Select | QItemSelectionModel::Rows);
            }
        }
    }
    for (QModelIndex index : deselected.indexes()) {
        ThermalNode* node = reinterpret_cast<ThermalNode*>(index.internalPointer());
        ThermalPreviewItem* item = node->item();
        if (item)
            item->setSelected(false);
        else {
            for (int i = 0; i < node->childCount(); ++i) {
                ui->treeView->selectionModel()->select(model->createIndex(i, 0, node->child(i)), QItemSelectionModel::Clear | QItemSelectionModel::Rows);
            }
        }
    }
}

void ThermalForm::redraw()
{
    for (QSharedPointer<ThermalPreviewItem> item : m_sourcePreview) {
        item->redraw();
    }
}

void ThermalForm::on_dsbxDepth_valueChanged(double arg1)
{
    m_depth = arg1;
    redraw();
}
