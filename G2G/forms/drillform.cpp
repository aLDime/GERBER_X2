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
enum { Size = 24 };

/////////////////////////////////////////////
/// \brief draw
/// \param aperture
/// \return
///
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
    m_apertures = value;
    model = new DrillModel(m_isAperture, this);
    QMap<int, QSharedPointer<G::AbstractAperture>>::const_iterator apertureIt;
    for (apertureIt = m_apertures.begin(); apertureIt != m_apertures.end(); ++apertureIt) {
        if (apertureIt.value()->isFlashed()) {
            double drillDiameter = 0.0;
            QString name(apertureIt.value()->name());
            if (apertureIt.value()->isDrilled()) {
                drillDiameter = apertureIt.value()->drillDiameter();
                name += QString(", drill Ø%1mm").arg(drillDiameter);
            }

            model->appendRow(name, draw(apertureIt.value().data()), apertureIt.key());

            const G::File* file = static_cast<G::File*>(ui->cbxFile->currentData().value<void*>());
            for (const G::GraphicObject& go : *file) {
                if (go.state.dCode == G::D03 && go.state.aperture == apertureIt.key()) {
                    G::State state(go.state);
                    state.curPos = IntPoint();

                    QPainterPath painterPath;
                    for (QPolygonF& polygon : PathsToQPolygons(m_apertures[apertureIt.key()]->draw(state)))
                        painterPath.addPolygon(polygon);
                    painterPath.addEllipse(QPointF(0, 0), m_apertures[apertureIt.key()]->drillDiameter() * 0.5, m_apertures[apertureIt.key()]->drillDiameter() * 0.5);

                    QGraphicsPathItem* item = new QGraphicsPathItem(painterPath);

                    setColor(item, Qt::darkGray);
                    item->setPos(ToQPointF(go.state.curPos));
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
    m_isAperture = false;
    clear();

    m_tools = value;
    model = new DrillModel(m_isAperture, this);

    QMap<int, double>::const_iterator apertureIt;
    for (apertureIt = m_tools.begin(); apertureIt != m_tools.end(); ++apertureIt) {
        QString name(QString("Tool Ø%1mm").arg(apertureIt.value()));

        QPixmap pixmap(Size, Size);
        pixmap.fill(Qt::transparent);
        QPainter painter;
        painter.begin(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);
        painter.drawEllipse(QRect(0, 0, Size - 1, Size - 1));
        QIcon icon(pixmap);

        model->appendRow(name, icon, apertureIt.key());

        const DrillFile* file = static_cast<DrillFile*>(ui->cbxFile->currentData().value<void*>());
        for (const Hole& hole : *file) {
            if (hole.state.tCode == apertureIt.key()) {
                QPainterPath painterPath;
                painterPath.addEllipse(QPointF(0, 0), hole.state.currentToolDiameter * 0.5, hole.state.currentToolDiameter * 0.5);
                QGraphicsPathItem* item = new QGraphicsPathItem(painterPath);

                setColor(item, Qt::darkGray);
                item->setPos(hole.state.pos);
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

void DrillForm::updateFiles()
{
    ui->cbxFile->clear();
    for (G::File* file : FileHolder::files<G::File>()) {
        for (const G::GraphicObject& go : *file) {
            if (go.state.dCode == G::D03) {
                ui->cbxFile->addItem(file->shortFileName(), QVariant::fromValue(static_cast<void*>(file)));
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
        ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QIcon::fromTheme("roll"), Qt::DecorationRole);
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
        ToolDatabase tdb(this, { Tool::Drill, Tool::EndMill });
        if (tdb.exec()) {
            int apertureId = model->apertureId(current.row());
            const Tool tool(tdb.tool());
            model->setToolId(current.row(), tool.id);
            if (!m_gid.contains(apertureId)) {
                createHoles(apertureId, tool.diameter);
            } else {
                for (DrillItem* item : m_gid[apertureId])
                    item->setDiameter(tool.diameter);
            }
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
        static_cast<QDockWidget*>(parent())->hide();
}

void DrillForm::on_pbCreate_clicked()
{
    QMap<int, Path> pos;
    QMap<int, QVector<int>> apetrureId;

    for (int row = 0; row < model->rowCount(); ++row) {
        int tool = model->toolId(row);
        int dNum = model->apertureId(row);
        if (tool != -1) {
            apetrureId[tool].append(dNum);
            for (DrillItem* item : m_gid[dNum]) {
                QPointF p(item->pos());
                pos[tool].append(IntPoint(p.x() * uScale, p.y() * uScale));
            }
        }
    }

    QMap<int, Path>::iterator iterator;
    for (iterator = pos.begin(); iterator != pos.end(); ++iterator) {
        Path src(pos[iterator.key()]);
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
        GCodeFile* gcode = new GCodeFile({ dst }, ToolDatabase::tools[iterator.key()], ui->dsbxDepth->value(), Drilling);
        QString str;
        for (int id : apetrureId[iterator.key()])
            str += (m_isAperture ? "D" : "T") + QString::number(id) + ", ";
        str.remove(str.size() - 2, 2);

        gcode->setFileName(ToolDatabase::tools[iterator.key()].name + " (" + str + ")");
        gcode->setSide(static_cast<G::File*>(ui->cbxFile->currentData().value<void*>())->side);
        FileModel::self->addGcode(gcode);
    }
}

void DrillForm::createHoles(int apertureId, double diameter)
{
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

void DrillForm::pickUpTool(int apertureId, double diameter)
{
    const double drillDiameterMin = diameter * 0.99;
    const double drillDiameterMax = diameter * 1.01;
    QMap<int, Tool>::const_iterator toolIt;
    for (toolIt = ToolDatabase::tools.begin(); toolIt != ToolDatabase::tools.end(); ++toolIt) {
        if (toolIt.value().type == Tool::Drill || toolIt.value().type == Tool::EndMill) {
            if (drillDiameterMin <= toolIt.value().diameter && toolIt.value().diameter <= drillDiameterMax) {
                qDebug() << toolIt.value().name;
                model->setToolId(model->rowCount() - 1, toolIt.key());
                createHoles(apertureId, toolIt.value().diameter);
                break;
            }
        }
    }
}
/////////////////////////////////////////////
/// \brief DrillModel::appendRow
/// \param name
/// \param icon
/// \param id
///
void DrillModel::appendRow(const QString& name, const QIcon& icon, int id)
{
    m_data.append(Row(name, icon, id));
}

int DrillModel::rowCount(const QModelIndex& /*parent*/) const { return m_data.size(); }

int DrillModel::columnCount(const QModelIndex& /*parent*/) const { return 2; }

QVariant DrillModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    if (!index.column())
        switch (role) {
        case Qt::DisplayRole:
            return m_data[row].name[0];
        case Qt::DecorationRole:
            return m_data[row].icon[0];
        case Qt::UserRole:
            return m_data[row].id[0];
        default:
            break;
        }
    else {
        if (m_data[row].id[1] == -1)
            switch (role) {
            case Qt::DisplayRole:
                return "Select Tool";
            case Qt::TextAlignmentRole:
                return Qt::AlignCenter;
            case Qt::UserRole:
                return m_data[row].id[index.column()];
            default:
                break;
            }
        else
            switch (role) {
            case Qt::DisplayRole:
                return ToolDatabase::tools[m_data[row].id[1]].name;
            case Qt::DecorationRole:
                return ToolDatabase::tools[m_data[row].id[1]].icon();
            case Qt::UserRole:
                return m_data[row].id[index.column()];
            default:
                break;
            }
    }
    return QVariant();
}

QVariant DrillModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return "Aperture";
            case 1:
                return "Tool";
            }

        } else {
            return (m_isAperture ? "D" : "T") + QString::number(m_data[section].id[0]);
        }
    case Qt::TextAlignmentRole:
        return Qt::AlignCenter;
    default:
        return QVariant();
    }
}
