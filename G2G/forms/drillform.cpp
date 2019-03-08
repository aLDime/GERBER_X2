#include "drillform.h"
#include "materialsetupform.h"
#include "ui_drillform.h"

#include <QMenu>
#include <QPainter>
#include <QTimer>
#include <file.h>
#include <myscene.h>

#include "filetree/filemodel.h"

#include "tooldatabase/tooldatabase.h"

#include "staticholders/fileholder.h"

#include "drillmodel.h"

DrillForm* DrillForm::self = nullptr;
enum { Size = 24 };

//int AbstractFileId = qRegisterMetaType<AbstractFile*>("AbstractFile*");

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
/// \brief setColor
/// \param item
/// \param color
///
void setColor(QGraphicsPathItem* item, Qt::GlobalColor color)
{
    item->setPen(QPen(color, 0.0));
    item->setBrush(color);
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
                });
                break;
            }
        }

        menu.exec(ui->tableView->mapToGlobal(pos /*+ QPoint(24, 24)*/));
    });

    ui->tableView->setWordWrap(false);
    updateFiles();
    self = this;
}

DrillForm::~DrillForm()
{
    qDebug("~DrillForm()");
    self = nullptr;
    //    if (MyScene::self)
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

                    setColor(item, Qt::darkGray);
                    item->setPos(toQPointF(go.state.curPos()));
                    m_giaperture[apertureIt.key()].append(item);
                    MyScene::self->addItem(item);
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
        for (const Hole& hole : *file) {
            if (hole.state.tCode == apertureIt.key()) {
                QPainterPath painterPath;
                painterPath.addEllipse(QPointF(0, 0), hole.state.currentToolDiameter() * 0.5, hole.state.currentToolDiameter() * 0.5);
                QGraphicsPathItem* item = new QGraphicsPathItem(painterPath);

                setColor(item, Qt::darkGray);
                item->setPos(hole.state.pos + file->format().offsetPos);
                item->setZValue(FileHolder::size());

                m_giaperture[apertureIt.key()].append(item);
                MyScene::self->addItem(item);
            }
        }

        if (apertureIt.value() != 0.0)
            pickUpTool(apertureIt.key(), apertureIt.value());
    }

    delete ui->tableView->model();
    ui->tableView->setModel(model);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &DrillForm::on_currentChanged);
}

void DrillForm::setItems()
{
    m_type = tItem;
    clear();
    model = new DrillModel(m_type, this);

    for (QGraphicsItem* gi : m_items) {
        GraphicsItem* it = dynamic_cast<GraphicsItem*>(gi);
        if (!it)
            continue;
        int id = m_items.indexOf(gi);
        QString name(QString("Item %1").arg(id));
        model->appendRow(name, drawDrillIcon(), id);

        QPainterPath painterPath;
        //        double d = 0.5;
        //        painterPath.addEllipse(QPointF(0, 0), d * 0.5, d * 0.5);
        painterPath.addRect(-0.01, -0.5, 0.02, 1);
        painterPath.addRect(-0.5, -0.01, 1, 0.02);
        QGraphicsPathItem* item = new QGraphicsPathItem(painterPath);

        setColor(item, Qt::darkGray);
        item->setPos(it->center());
        item->setZValue(FileHolder::size());

        m_giaperture[id].append(item);
        MyScene::self->addItem(item);
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
            QColor color(file->itemGroup()->brush().color());
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

    m_items = MyScene::self->selectedItems();

    if (!m_items.isEmpty()) {
        ui->cbxFile->addItem("Selected");
        ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QSize(0, Size), Qt::SizeHintRole);
        //    ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QIcon::fromTheme("roll"), Qt::DecorationRole);
    }

    if (!ui->cbxFile->count()) {
        QMessageBox::information(this, "", "No data to process.");
        QTimer::singleShot(1, Qt::CoarseTimer, [=] { on_pbClose_clicked(); });
    } else
        on_cbxFile_currentIndexChanged(0);
}

void DrillForm::on_cbxFile_currentIndexChanged(int index)
{
    if (ui->cbxFile->count() == index) {
        //? connect(MyScene::self, &MyScene::selectionChanged, this, &DrillForm::on_selection_changed);
        setItems();
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
        ToolDatabase tdb(this, { Tool::Drill, Tool::EndMill });
        if (tdb.exec()) {
            int apertureId = model->apertureId(current.row());
            const Tool tool(tdb.tool());
            model->setToolId(current.row(), tool.id);
            createHoles(apertureId, tool.diameter);
        }
    }
}

void DrillForm::on_currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    if (previous.isValid() && previous.row() != current.row()) {
        int apertureId = model->apertureId(current.row());
        for (QGraphicsPathItem* item : m_giaperture[apertureId])
            setColor(item, Qt::magenta);
    }
    if (previous.isValid() && previous.row() != current.row()) {
        int apertureId = model->apertureId(previous.row());
        for (QGraphicsPathItem* item : m_giaperture[apertureId])
            setColor(item, Qt::darkGray);
    }
}

void DrillForm::clear()
{
    for (QVector<QGraphicsPathItem*>& vector : m_giaperture)
        qDeleteAll(vector);
    m_giaperture.clear();
    for (QVector<DrillItem*>& vector : m_gid)
        qDeleteAll(vector);
    m_gid.clear();
}

void DrillForm::on_pbClose_clicked()
{
    if (parent())
        static_cast<QWidget*>(parent())->close();
}

void DrillForm::on_pbCreate_clicked()
{
    QMap<int, QPair<Path, QVector<int>>> paths;

    for (int row = 0; row < model->rowCount(); ++row) {

        int toolId = model->toolId(row);
        if (toolId != -1) {
            int apertureId = model->apertureId(row);
            paths[toolId].second.append(apertureId);

            Path& path = paths[toolId].first;
            for (DrillItem* item : m_gid[apertureId]) {
                path.append(toIntPoint(item->pos()));
            }
        }
    }

    QMap<int, QPair<Path, QVector<int>>>::iterator iterator;
    for (iterator = paths.begin(); iterator != paths.end(); ++iterator) {
        int toolId = iterator.key();
        Path& path = paths[toolId].first;
        IntPoint point1(toIntPoint(MaterialSetup::homePos));
        int counter = 0;
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
        GCodeFile* gcode;
        if (m_type == tItem) {
            gcode = new GCodeFile({ path }, ToolHolder::tools[toolId], ui->dsbxDepth->value(), Drilling);
            gcode->setFileName(ToolHolder::tools[toolId].name + " (Items)");
        } else {
            QString indexes;
            QVector<int>& v = paths[toolId].second;
            for (int id : v)
                indexes += QString::number(id) + (id != v.last() ? ", " : "");

            gcode = new GCodeFile({ path }, ToolHolder::tools[toolId], ui->dsbxDepth->value(), Drilling);
            gcode->setFileName(ToolHolder::tools[toolId].name + (m_type ? " - D(" : " - T(") + indexes + ')');
            gcode->setSide(static_cast<AbstractFile*>(ui->cbxFile->currentData().value<void*>())->side());
        }

        FileModel::self->addGcode(gcode);
    }
}

void DrillForm::createHoles(int apertureId, double diameter)
{
    if (m_gid.contains(apertureId)) {
        for (DrillItem* item : m_gid[apertureId])
            item->setDiameter(diameter);
    } else {
        for (const QGraphicsPathItem* itemA : m_giaperture[apertureId]) {
            DrillItem* item = new DrillItem(diameter);
            item->setPen(Qt::NoPen);
            item->setBrush(Qt::red);
            item->setZValue(itemA->zValue());
            item->setPos(itemA->pos());
            m_gid[apertureId].append(item);
            MyScene::self->addItem(item);
        }
    }
}

void DrillForm::removeHoles(int apertureId)
{
    qDeleteAll(m_gid[apertureId]);
    m_gid.remove(apertureId);
}

void DrillForm::pickUpTool(int apertureId, double diameter)
{
    const double k = 0.05; //5%
    const double drillDiameterMin = diameter * (1.0 - k);
    const double drillDiameterMax = diameter * (1.0 + k);
    QMap<int, Tool>::const_iterator toolIt;
    for (toolIt = ToolHolder::tools.begin(); toolIt != ToolHolder::tools.end(); ++toolIt) {
        if (toolIt.value().type == Tool::Drill
            && drillDiameterMin <= toolIt.value().diameter
            && drillDiameterMax >= toolIt.value().diameter) {
            model->setToolId(model->rowCount() - 1, toolIt.key());
            createHoles(apertureId, toolIt.value().diameter);
            return;
        }
    }
    for (toolIt = ToolHolder::tools.begin(); toolIt != ToolHolder::tools.end(); ++toolIt) {
        if (toolIt.value().type == Tool::EndMill
            && drillDiameterMin <= toolIt.value().diameter
            && drillDiameterMax >= toolIt.value().diameter) {
            model->setToolId(model->rowCount() - 1, toolIt.key());
            createHoles(apertureId, toolIt.value().diameter);
            return;
        }
    }
}

void DrillForm::on_selection_changed()
{
    qDebug("selectionChanged");
}
