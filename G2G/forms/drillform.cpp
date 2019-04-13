#include "drillform.h"
#include "drillmodel.h"
#include "filetree/fileholder.h"
#include "filetree/filemodel.h"
#include "materialsetupform.h"
#include "tooldatabase/tooldatabase.h"
#include "ui_drillform.h"
#include <QMenu>
#include <QPainter>
#include <QTimer>
#include <file.h>
#include <scene.h>

DrillForm* DrillForm::self = nullptr;
enum { Size = 24 };

/////////////////////////////////////////////
/// \brief draw
/// \param aperture
/// \return
///
QIcon drawApertureIcon(G::AbstractAperture* aperture)
{
    QPainterPath painterPath;

    for (QPolygonF& polygon : toQPolygons(aperture->draw(G::State())))
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
    return QIcon(pixmap);
}

QIcon drawDrillIcon()
{
    QPixmap pixmap(Size, Size);
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.drawEllipse(QRect(0, 0, Size - 1, Size - 1));
    return QIcon(pixmap);
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
    connect(ui->tableView, &QTableView::clicked, this, &DrillForm::on_clicked);

    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, &QTableView::customContextMenuRequested, [&](const QPoint& pos) {
        if (ui->tableView->selectionModel()->selectedIndexes().isEmpty())
            return;
        QMenu menu;
        menu.addAction(QIcon::fromTheme("view-form"), tr("&Select Tool"), [=] {
            ToolDatabase tdb(this, { Tool::Drill, Tool::EndMill });
            if (tdb.exec()) {
                const Tool tool(tdb.tool());
                for (QModelIndex current : ui->tableView->selectionModel()->selectedIndexes()) {
                    model->setToolId(current.row(), tool.id);
                    createHoles(model->apertureId(current.row()), tool.diameter);
                    ui->pbCreate->setEnabled(true);
                }
            }
        });

        for (QModelIndex current : ui->tableView->selectionModel()->selectedIndexes()) {
            if (model->toolId(current.row()) != -1) {
                menu.addAction(QIcon::fromTheme("list-remove"), tr("&Remove Tool"), [=] {
                    for (QModelIndex current : ui->tableView->selectionModel()->selectedIndexes()) {
                        model->setToolId(current.row(), -1);
                        removeHoles(model->apertureId(current.row()));
                    }
                    for (int i = 0; i < model->rowCount(); ++i) {
                        if (model->toolId(i) != -1)
                            return;
                    }
                    ui->pbCreate->setEnabled(false);
                });
                break;
            }
        }

        menu.exec(ui->tableView->mapToGlobal(pos /*+ QPoint(24, 24)*/));
    });

    ui->tableView->setWordWrap(false);
    ui->pbCreate->setEnabled(false);
    updateFiles();
    self = this;
}

DrillForm::~DrillForm()
{
    self = nullptr;
    clear();
    delete ui;
}

void DrillForm::setApertures(const QMap<int, QSharedPointer<G::AbstractAperture>>& value)
{
    m_type = tAperture;
    clear();
    m_apertures = value;
    model = new DrillModel(m_type, this);
    QMap<int, QSharedPointer<G::AbstractAperture>>::const_iterator apertureIt;
    for (apertureIt = m_apertures.begin(); apertureIt != m_apertures.end(); ++apertureIt) {
        if (apertureIt.value()->isFlashed()) {
            double drillDiameter = 0.0;
            QString name(apertureIt.value()->name());
            if (apertureIt.value()->isDrilled()) {
                drillDiameter = apertureIt.value()->drillDiameter();
                name += QString(", drill Ø%1mm").arg(drillDiameter);
            } else if (apertureIt.value()->type() == G::Circle) {
                drillDiameter = apertureIt.value()->apertureSize();
            }

            model->appendRow(name, drawApertureIcon(apertureIt.value().data()), apertureIt.key());

            const G::File* file = static_cast<G::File*>(ui->cbxFile->currentData().value<void*>());
            for (const G::GraphicObject& go : *file) {
                if (go.state.dCode() == G::D03 && go.state.aperture() == apertureIt.key()) {
                    G::State state(go.state);
                    state.curPos() = IntPoint();

                    QPainterPath painterPath;
                    for (QPolygonF& polygon : toQPolygons(m_apertures[apertureIt.key()]->draw(state)))
                        painterPath.addPolygon(polygon);
                    painterPath.addEllipse(QPointF(0, 0), m_apertures[apertureIt.key()]->drillDiameter() * 0.5, m_apertures[apertureIt.key()]->drillDiameter() * 0.5);

                    QGraphicsPathItem* item = new QGraphicsPathItem(painterPath);
                    item->setPen(QPen(Qt::darkGray, 0.0));
                    item->setPos(toQPointF(go.state.curPos()));
                    item->setBrush(Qt::darkGray);
                    m_drills[apertureIt.key()].append(item->pos());
                    m_sourcePreview[apertureIt.key()].append(item);
                    Scene::self->addItem(item);
                }
            }

            if (drillDiameter != 0.0)
                pickUpTool(apertureIt.key(), drillDiameter);
        }
    }

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
    m_type = tTool;
    clear();

    m_tools = value;
    model = new DrillModel(m_type, this);

    QMap<int, double>::const_iterator apertureIt;
    for (apertureIt = m_tools.begin(); apertureIt != m_tools.end(); ++apertureIt) {
        QString name(QString("Tool Ø%1mm").arg(apertureIt.value()));
        model->appendRow(name, drawDrillIcon(), apertureIt.key());
        const DrillFile* file = static_cast<DrillFile*>(ui->cbxFile->currentData().value<void*>());
        bool isSlot = false;
        for (const Hole& hole : *file) {
            if (hole.state.tCode == apertureIt.key()) {
                QGraphicsPathItem* item = nullptr;
                if (hole.state.path.isEmpty()) {
                    QPainterPath painterPath;
                    painterPath.addEllipse(hole.state.pos, hole.state.currentToolDiameter() * 0.5, hole.state.currentToolDiameter() * 0.5);
                    item = new QGraphicsPathItem(painterPath);
                    item->setPen(QPen(Qt::darkGray, 0.0));
                    m_drills[apertureIt.key()].append(hole.state.pos + file->format().offsetPos);

                } else {
                    QPainterPath painterPath;
                    painterPath.addPolygon(hole.state.path);
                    item = new QGraphicsPathItem(painterPath);
                    item->setPen(QPen(Qt::darkGray, hole.state.currentToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    m_slots[apertureIt.key()].append(hole.state.path.translated(file->format().offsetPos));
                    isSlot = true;
                }
                item->setBrush(Qt::darkGray);
                item->setZValue(FileHolder::size());
                item->setPos(file->format().offsetPos);
                m_sourcePreview[apertureIt.key()].append(item);
                Scene::self->addItem(item);
            }
        }
        model->setSlot(model->rowCount() - 1, isSlot);
        if (apertureIt.value() != 0.0)
            pickUpTool(apertureIt.key(), apertureIt.value(), isSlot);
    }

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
    if (ui->tableView->model())
        delete ui->tableView->model();
    ui->tableView->setModel(new DrillModel(m_type, this));
    clear();

    ui->cbxFile->clear();

    for (G::File* file : FileHolder::files<G::File>()) {
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

    for (DrillFile* file : FileHolder::files<DrillFile>()) {
        ui->cbxFile->addItem(file->shortFileName(), QVariant::fromValue(static_cast<void*>(file)));
        ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QIcon::fromTheme("roll"), Qt::DecorationRole);
        ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QSize(0, Size), Qt::SizeHintRole);
    }

    if (!ui->cbxFile->count()) {
        QMessageBox::information(this, "", "No data to process.");
        QTimer::singleShot(1, Qt::CoarseTimer, [=] { on_pbClose_clicked(); });
    } else
        on_cbxFile_currentIndexChanged(0);
}

void DrillForm::on_cbxFile_currentIndexChanged(int index)
{
    ui->pbCreate->setEnabled(false);
    if (ui->cbxFile->count() == index) {
        return; // setItems();
    } else if (ui->cbxFile->count()) {
        if (static_cast<AbstractFile*>(ui->cbxFile->currentData().value<void*>())->type() == FileType::Gerber)
            setApertures(static_cast<G::File*>(ui->cbxFile->currentData().value<void*>())->apertures());
        else
            setHoles(static_cast<DrillFile*>(ui->cbxFile->currentData().value<void*>())->tools());
    }
}

void DrillForm::on_doubleClicked(const QModelIndex& current)
{
    if (current.column() == 1) {
        ToolDatabase tdb(this, model->isSlot(current.row()) ? QVector<Tool::Type>{ Tool::EndMill } : QVector<Tool::Type>{ Tool::Drill, Tool::EndMill });
        if (tdb.exec()) {
            int apertureId = model->apertureId(current.row());
            const Tool tool(tdb.tool());
            model->setToolId(current.row(), tool.id);
            createHoles(apertureId, tool.diameter);
        }
    }
}

void DrillForm::on_clicked(const QModelIndex& index)
{
    int apertureId = model->apertureId(index.row());
    for (QGraphicsPathItem* item : m_sourcePreview[apertureId])
        item->setBrush(Qt::green);
}

void DrillForm::on_currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    if (previous.isValid() && previous.row() != current.row()) {
        int apertureId = model->apertureId(current.row());
        for (QGraphicsPathItem* item : m_sourcePreview[apertureId])
            item->setBrush(Qt::green);
    }
    if (previous.isValid() && previous.row() != current.row()) {
        int apertureId = model->apertureId(previous.row());
        for (QGraphicsPathItem* item : m_sourcePreview[apertureId])
            item->setBrush(Qt::darkGray);
    }
}

void DrillForm::clear()
{
    for (QVector<QGraphicsPathItem*>& vector : m_sourcePreview)
        qDeleteAll(vector);
    m_sourcePreview.clear();
    m_drills.clear();
    m_slots.clear();

    for (QVector<DrillItem*>& vector : m_giDrill)
        qDeleteAll(vector);
    m_giDrill.clear();
}

void DrillForm::on_pbClose_clicked()
{
    if (parent())
        static_cast<QWidget*>(parent())->close();
}

void DrillForm::on_pbCreate_clicked()
{
    QMap<int, QPair<Paths, QVector<int>>> pathsMap;

    for (int row = 0; row < model->rowCount(); ++row) {
        int toolId = model->toolId(row);
        if (toolId != -1) {
            int apertureId = model->apertureId(row);
            pathsMap[toolId].second.append(apertureId);
            for (DrillItem* item : m_giDrill[apertureId]) {
                if (pathsMap[toolId].first.isEmpty())
                    pathsMap[toolId].first.append(Path());
                if (item->isSlot())
                    pathsMap[toolId].first.append(item->paths().first());
                else
                    pathsMap[toolId].first.first().append(toIntPoint(item->pos()));
            }
        }
    }

    QMap<int, QPair<Paths, QVector<int>>>::iterator iterator;
    for (iterator = pathsMap.begin(); iterator != pathsMap.end(); ++iterator) {
        int toolId = iterator.key();
        Path& path = pathsMap[toolId].first.first();
        IntPoint point1(toIntPoint(MaterialSetup::homePos));
        int counter = 0;
        { // sort by distance
            while (counter < path.size()) {
                int selector = 0;
                double length = std::numeric_limits<double>::max();
                for (int i = counter, end = path.size(); i < end; ++i) {
                    double length2 = Length(point1, path[i]);
                    if (length > length2) {
                        length = length2;
                        selector = i;
                    }
                }
                qSwap(path[counter], path[selector]);
                point1 = path[counter++];
            }
        }
        GCodeFile* gcode;
        QString indexes;
        QVector<int>& v = pathsMap[toolId].second;
        for (int id : v)
            indexes += QString::number(id) + (id != v.last() ? "," : "");
        if (!path.isEmpty()) {
            gcode = new GCodeFile({ path }, ToolHolder::tools[toolId], ui->dsbxDepth->value(), Drilling);
            gcode->setFileName(ToolHolder::tools[toolId].name + (m_type ? " - T(" : " - D(") + indexes + ')');
            gcode->setSide(static_cast<AbstractFile*>(ui->cbxFile->currentData().value<void*>())->side());
            FileModel::self->addGcode(gcode);
        }
        if (pathsMap[toolId].first.size() > 1) {
            pathsMap[toolId].first.removeFirst();
            //            for (int i = 0; i < pathsMap[toolId].first.size(); ++i) {
            //            }
            gcode = new GCodeFile(pathsMap[toolId].first, ToolHolder::tools[toolId], ui->dsbxDepth->value(), Profile);
            gcode->setFileName(ToolHolder::tools[toolId].name + " - T(" + indexes + ")(Slot)");
            gcode->setSide(static_cast<AbstractFile*>(ui->cbxFile->currentData().value<void*>())->side());
            FileModel::self->addGcode(gcode);
        }
    }
}

void DrillForm::createHoles(int toolId, double diameter)
{
    if (m_giDrill.contains(toolId)) {
        for (DrillItem* item : m_giDrill[toolId])
            item->setDiameter(diameter);
    } else {

        for (QPointF pos : m_drills[toolId]) {
            DrillItem* item = new DrillItem(diameter);
            item->setPen(QPen(Qt::red, 0.0));
            //item->setBrush(QBrush(Qt::red, Qt::Dense4Pattern));
            item->setBrush(QBrush(QColor(255, 0, 0, 150)));
            item->setZValue(std::numeric_limits<double>::max());
            item->setPos(pos);
            m_giDrill[toolId].append(item);
            Scene::self->addItem(item);
        }
        for (QPolygonF path : m_slots[toolId]) {
            DrillItem* item = new DrillItem(path, diameter);
            item->setPen(QPen(Qt::red, 0.0));
            //item->setBrush(QBrush(Qt::red, Qt::Dense4Pattern));
            item->setBrush(QBrush(QColor(255, 0, 0, 150)));
            item->setZValue(std::numeric_limits<double>::max());
            //item->setPos(path.first());
            m_giDrill[toolId].append(item);
            Scene::self->addItem(item);
        }
    }
}

void DrillForm::removeHoles(int apertureId)
{
    qDeleteAll(m_giDrill[apertureId]);
    m_giDrill.remove(apertureId);
}

void DrillForm::pickUpTool(int apertureId, double diameter, bool isSlot)
{
    const double k = 0.05; // 5%
    const double drillDiameterMin = diameter * (1.0 - k);
    const double drillDiameterMax = diameter * (1.0 + k);
    QMap<int, Tool>::const_iterator toolIt;
    for (toolIt = ToolHolder::tools.begin(); !isSlot && toolIt != ToolHolder::tools.end(); ++toolIt) {
        if (toolIt.value().type == Tool::Drill
            && drillDiameterMin <= toolIt.value().diameter
            && drillDiameterMax >= toolIt.value().diameter) {
            model->setToolId(model->rowCount() - 1, toolIt.key());
            createHoles(apertureId, toolIt.value().diameter);
            ui->pbCreate->setEnabled(true);
            return;
        }
    }
    for (toolIt = ToolHolder::tools.begin(); toolIt != ToolHolder::tools.end(); ++toolIt) {
        if (toolIt.value().type == Tool::EndMill
            && drillDiameterMin <= toolIt.value().diameter
            && drillDiameterMax >= toolIt.value().diameter) {
            model->setToolId(model->rowCount() - 1, toolIt.key());
            createHoles(apertureId, toolIt.value().diameter);
            ui->pbCreate->setEnabled(true);
            return;
        }
    }
}
