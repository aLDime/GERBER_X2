#include "drillform.h"
#include "materialsetupform.h"
#include "ui_drillform.h"

#include <QDebug>
#include <QDockWidget>
#include <QFileInfo>
#include <QMessageBox>
#include <QPainter>
#include <QStandardItemModel>
#include <file.h>
#include <myscene.h>

#include "tooldatabase/tooldatabase.h"
#include <filetree/gerbernode.h>

#include <filetree/FileHolder.h>
#include <filetree/filemodel.h>

using namespace ClipperLib;

DrillForm* DrillForm::self = nullptr;
enum {
    Size = 20,
    D_NumberRole = Qt::UserRole + 1,
    ToolRole = Qt::UserRole + 1
};

QIcon draw(G::AbstractAperture* aperture)
{
    QPainterPath painterPath;

    for (QPolygonF& polygon : PathsToQPolygons(aperture->draw(G::State())))
        painterPath.addPolygon(polygon);

    painterPath.addEllipse(QPointF(0, 0), aperture->drillDiameter() * 0.5, aperture->drillDiameter() * 0.5);

    const QRectF rect = painterPath.boundingRect();

    qreal scale = (double)Size / qMax(rect.width(), rect.height());

    double ky = -rect.top() * scale;
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
    painter.scale(scale, scale);
    painter.drawPath(painterPath);
    QIcon icon(pixmap);
    return icon;
}

/////////////////////////////////////////////
/// \brief DrillForm::DrillForm
/// \param parent
///
DrillForm::DrillForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::DrillForm)
{
    ui->setupUi(this);
    ui->tableView->setIconSize(QSize(Size, Size));
    ui->dsbxDepth->setValue(MaterialSetup::thickness);
    connect(ui->tableView, &QTableView::doubleClicked, this, &DrillForm::on_doubleClicked);
    updateFiles();
    self = this;
}

DrillForm::~DrillForm()
{
    self = nullptr;
    if (MyScene::self)
        clear();
    delete ui;
}

void DrillForm::setApertures(const QMap<int, QSharedPointer<G::AbstractAperture>>& value)
{
    m_isAperture = true;
    clear();

    apertures = value;
    model = new QStandardItemModel(this);

    QMapIterator<int, QSharedPointer<G::AbstractAperture>> i(apertures);
    QString verticalHeaderLabels;
    while (i.hasNext()) {
        i.next();
        if (i.value()->isFlashed()) {
            verticalHeaderLabels.append("D" + QString::number(i.key()) + "|");
            QString name(i.value()->name());
            if (i.value()->isDrilled())
                name += QString(", drill Ø%1mm").arg(i.value()->drillDiameter());
            QStandardItem* item1 = new QStandardItem(draw(i.value().data()), name);
            item1->setFlags(Qt::ItemIsEnabled);
            item1->setData(i.key(), D_NumberRole);

            QStandardItem* item2 = new QStandardItem("Select Drill");
            item2->setFlags(Qt::ItemIsEnabled);
            item2->setData(-1, ToolRole);

            model->appendRow({ item1, item2 });
        }
    }

    model->setHorizontalHeaderLabels(QString("Aperture|Tool").split('|'));
    model->setVerticalHeaderLabels(verticalHeaderLabels.split('|', QString::SkipEmptyParts));

    delete ui->tableView->model();
    ui->tableView->setModel(model);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &DrillForm::on_currentChanged);
}

void DrillForm::setHoles(const QMap<int, double>& value)
{
    m_isAperture = false;
    clear();

    tools = value;
    model = new QStandardItemModel(this);

    QMapIterator<int, double> i(tools);
    QString verticalHeaderLabels;
    while (i.hasNext()) {
        i.next();
        verticalHeaderLabels.append("T" + QString::number(i.key()) + "|");
        QString name(QString("Tool Ø%1mm").arg(i.value()));

        QPixmap pixmap(Size, Size);
        pixmap.fill(Qt::transparent);
        QPainter painter;
        painter.begin(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);
        painter.drawEllipse(QRect(0, 0, Size - 1, Size - 1));
        QIcon icon(pixmap);

        QStandardItem* item1 = new QStandardItem(icon, name);
        item1->setFlags(Qt::ItemIsEnabled);
        item1->setData(i.key(), D_NumberRole);

        QStandardItem* item2 = new QStandardItem("Select Drill");
        item2->setFlags(Qt::ItemIsEnabled);
        item2->setData(-1, ToolRole);

        model->appendRow({ item1, item2 });
    }

    model->setHorizontalHeaderLabels(QString("Aperture|Tool").split('|'));
    model->setVerticalHeaderLabels(verticalHeaderLabels.split('|', QString::SkipEmptyParts));

    delete ui->tableView->model();
    ui->tableView->setModel(model);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &DrillForm::on_currentChanged);
}

void DrillForm::updateFiles()
{
    ui->cbxFile->clear();
    for (G::File* file : FileHolder::files<G::File>()) {
        for (const G::GraphicObject& go : *file) {
            if (go.state.dCode == G::D03) {
                ui->cbxFile->addItem(file->shortFileName(), QVariant::fromValue(static_cast<void*>(file)));
                //ui->cbxFile->setItemData(ui->cbxFile->count() - 1, file->itemGroup()->brush().color(), Qt::BackgroundColorRole);
                QPixmap pixmap(Size, Size);
                QColor color(file->itemGroup()->brush().color());
                color.setAlpha(255);
                pixmap.fill(color);
                ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QIcon(pixmap), Qt::DecorationRole);
                ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QSize(0, Size), Qt::SizeHintRole);
                break;
            }
        }
    }
    for (DrillFile* file : FileHolder::files<DrillFile>()) {
        ui->cbxFile->addItem(file->shortFileName(), QVariant::fromValue(static_cast<void*>(file)));
        QPixmap pixmap(Size, Size);
        QColor color(file->itemGroup()->brush().color());
        color.setAlpha(255);
        pixmap.fill(color);
        ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QIcon(pixmap), Qt::DecorationRole);
        ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QSize(0, Size), Qt::SizeHintRole);
    }
}

void DrillForm::on_cbxFile_currentIndexChanged(int /*index*/)
{
    if (ui->cbxFile->count())
        if (static_cast<AbstractFile*>(ui->cbxFile->currentData().value<void*>())->type() == FileType::Gerber)
            setApertures(static_cast<G::File*>(ui->cbxFile->currentData().value<void*>())->getApertures());
        else
            setHoles(static_cast<DrillFile*>(ui->cbxFile->currentData().value<void*>())->m_toolDiameter);
}

void DrillForm::on_doubleClicked(const QModelIndex& current)
{

    if (current.column() == 1) {
        ToolDatabase tdb(this, { Tool::Drill });
        if (tdb.exec()) {
            int dNum = current.sibling(current.row(), 0).data(D_NumberRole).toInt();
            Tool tool(tdb.tool());
            QStandardItem* stdItem = static_cast<const QStandardItemModel*>(current.model())->itemFromIndex(current);
            stdItem->setData(tool.name, Qt::DisplayRole);
            stdItem->setData(QVariant::fromValue(tool), ToolRole);
            if (!gid.contains(dNum)) {
                for (QGraphicsPathItem* itemA : gia[dNum]) {

                    DrillItem* item = new DrillItem(tool.diameter);
                    item->setPen(Qt::NoPen);
                    item->setBrush(Qt::red);
                    item->setZValue(itemA->zValue());
                    item->setPos(itemA->pos());
                    gid[dNum].append(item);
                    MyScene::self->addItem(item);
                }
            } else {
                for (DrillItem* item : gid[dNum])
                    item->setDiameter(tool.diameter);
            }
        }
    }
}

void DrillForm::on_currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    auto setColor = [](QGraphicsPathItem* item, Qt::GlobalColor color) {
        item->setPen(QPen(color, 0.0));
        item->setBrush(color);
    };
    int dNum = current.sibling(current.row(), 0).data(D_NumberRole).toInt();
    if (!gia.contains(dNum)) {
        if (m_isAperture) {
            const G::File* file = static_cast<G::File*>(ui->cbxFile->currentData().value<void*>());
            for (const G::GraphicObject& go : *file) {
                if (go.state.dCode == G::D03 && go.state.aperture == dNum) {
                    G::State state(go.state);
                    state.curPos = IntPoint();

                    QPainterPath painterPath;
                    for (QPolygonF& polygon : PathsToQPolygons(apertures[dNum]->draw(state)))
                        painterPath.addPolygon(polygon);
                    painterPath.addEllipse(QPointF(0, 0), apertures[dNum]->drillDiameter() * 0.5, apertures[dNum]->drillDiameter() * 0.5);

                    QGraphicsPathItem* item = new QGraphicsPathItem(painterPath);

                    setColor(item, Qt::magenta);
                    item->setPos(ToQPointF(go.state.curPos));
                    //                item->setZValue(FileHolder::gerberFiles().lastKey() + 1);

                    gia[dNum].append(item);
                    MyScene::self->addItem(item);
                }
            }
        } else {
            const DrillFile* file = static_cast<DrillFile*>(ui->cbxFile->currentData().value<void*>());
            for (const Hole& hole : *file) {
                if (hole.state.tCode == dNum) {
                    QPainterPath painterPath;
                    painterPath.addEllipse(QPointF(0, 0), hole.state.currentToolDiameter * 0.5, hole.state.currentToolDiameter * 0.5);
                    QGraphicsPathItem* item = new QGraphicsPathItem(painterPath);

                    setColor(item, Qt::magenta);
                    item->setPos(hole.state.pos);
                    item->setZValue(FileHolder::size());

                    gia[dNum].append(item);
                    MyScene::self->addItem(item);
                }
            }
        }
    } else {
        if (previous.isValid() && previous.row() != current.row())
            for (QGraphicsPathItem* item : gia[dNum])
                setColor(item, Qt::magenta);
    }
    if (previous.isValid() && previous.row() != current.row()) {
        int dNumPrev = previous.sibling(previous.row(), 0).data(D_NumberRole).toInt();
        for (QGraphicsPathItem* item : gia[dNumPrev])
            setColor(item, Qt::darkGray);
    }
}

void DrillForm::clear()
{
    for (QVector<QGraphicsPathItem*>& vector : gia)
        qDeleteAll(vector);
    gia.clear();
    for (QVector<DrillItem*>& vector : gid)
        qDeleteAll(vector);
    gid.clear();
}

void DrillForm::on_pbClose_clicked()
{
    if (parent())
        static_cast<QDockWidget*>(parent())->hide();
}

void DrillForm::on_pbCreate_clicked()
{
    QMap<int, Tool> tools;
    QMap<int, Path> pos;
    QMap<int, QVector<int>> dCode;

    for (int row = 0; row < model->rowCount(); ++row) {
        Tool tool = model->item(row, 1)->data(ToolRole).value<Tool>();
        int dNum = model->item(row, 0)->data(D_NumberRole).toInt();
        if (tool.id > -1) {
            dCode[tool.id].append(dNum);
            for (DrillItem* item : gid[dNum]) {
                QPointF p(item->pos());
                pos[tool.id].append(IntPoint(p.x() * uScale, p.y() * uScale));
            }
            tools[tool.id] = tool;
        }
    }

    QMapIterator<int, Tool> i(tools);
    while (i.hasNext()) {
        i.next();
        Path src(pos[i.key()]);
        Path dst;
        dst.reserve(src.size());
        IntPoint p1(MaterialSetup::homePos.x() * uScale, MaterialSetup::homePos.y() * uScale);
        while (src.size()) {
            int s = 0;
            IntPoint p2;
            double l1 = Length(p1, p2);
            for (int i = 0; i < src.size(); ++i) {
                p2 = src[i];
                double l2 = Length(p1, p2);
                if (l1 > l2) {
                    l1 = l2;
                    s = i;
                }
            }
            dst.append(src.takeAt(s));
            p1 = dst.last();
        }
        GCodeFile* gcode = new GCodeFile({ dst }, {}, i.value(), ui->dsbxDepth->value(), Drilling);
        QString str;
        for (int d : dCode[i.key()]) {
            if (m_isAperture)
                str += "D" + QString::number(d) + ", ";
            else
                str += "T" + QString::number(d) + ", ";
        }
        str.remove(str.size() - 2, 2);
        gcode->setFileName(i.value().name + " (" + str + ")");
        gcode->setSide(static_cast<G::File*>(ui->cbxFile->currentData().value<void*>())->side);
        FileModel::self->addGcode(gcode);
    }
}
