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
#include <gcode/toolpathcreator.h>
#include <scene.h>

DrillForm* DrillForm::self = nullptr;
enum { Size = 24 };

Paths offset(const Path path, double offset, bool fl = false)
{
    Paths tmpPpaths;
    ClipperOffset cpOffset;
    if (fl)
        cpOffset.AddPath(path, jtRound, etClosedLine);
    else
        cpOffset.AddPath(path, jtRound, etOpenRound);
    cpOffset.Execute(tmpPpaths, offset * 0.5 * uScale);
    for (Path& path : tmpPpaths) {
        path.append(path.first());
    }
    return tmpPpaths;
}

class PreviewItem : public QGraphicsItem {
    static QPainterPath drawApetrure(const G::GraphicObject& go, int id)
    {
        QPainterPath painterPath;
        for (QPolygonF& polygon : toQPolygons(go.paths /*go.gFile->apertures()->value(id)->draw(go.state)*/))
            painterPath.addPolygon(polygon);
        const double hole = go.gFile->apertures()->value(id)->drillDiameter() * 0.5;
        if (hole)
            painterPath.addEllipse(toQPointF(go.state.curPos()), hole, hole);
        return painterPath;
    }
    static QPainterPath drawDrill(const Hole& hole)
    {
        QPainterPath painterPath;
        painterPath.addEllipse(hole.state.offsetPos(), hole.state.currentToolDiameter() * 0.5, hole.state.currentToolDiameter() * 0.5);
        return painterPath;
    }
    static QPainterPath drawSlot(const Hole& hole)
    {
        QPainterPath painterPath;
        for (Path& path : offset(toPath(hole.state.path), hole.state.currentToolDiameter())) {
            path.append(path.first());
            painterPath.addPolygon(toQPolygon(path));
        }
        return painterPath;
    }

public:
    explicit PreviewItem(const G::GraphicObject& go, int id)
        : id(id)
        , grob(&go)
        , m_sourcePath(drawApetrure(go, id))
        , m_sourceDrill(go.gFile->apertures()->value(id)->drillDiameter() ? go.gFile->apertures()->value(id)->drillDiameter() : go.gFile->apertures()->value(id)->apSize())
        , m_type(Apetrure)
        , m_pen(Qt::darkGray, 0.0)
        , m_brush(Qt::darkGray)
    {
    }
    explicit PreviewItem(const Hole& hole, const QPolygonF& toolPath)
        : hole(&hole)
        , m_sourcePath(drawSlot(hole))
        , m_sourceDrill(hole.state.currentToolDiameter())
        , m_toolPath(toolPath)
        , m_type(Slot)
        , m_pen(Qt::darkGray, 0.0)
        , m_brush(Qt::darkGray)
    {
    }
    explicit PreviewItem(const Hole& hole)
        : hole(&hole)
        , m_sourcePath(drawDrill(hole))
        , m_sourceDrill(hole.state.currentToolDiameter())
        , m_type(Drill)
        , m_pen(Qt::darkGray, 0.0)
        , m_brush(Qt::darkGray)
    {
    }

    ~PreviewItem() {}

    enum Type {
        Slot = ShtiftType + 1,
        Drill,
        Apetrure
    };

    // QGraphicsItem interface
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) override
    {
        // draw source
        painter->setPen(m_pen);
        painter->setBrush(m_brush);
        painter->drawPath(m_sourcePath);
        // draw hole
        if (!qFuzzyIsNull(m_currentDrill)) {
            //item->setBrush(QBrush(Qt::red, Qt::Dense4Pattern));
            painter->setPen(QPen(Qt::red, 0.0));
            painter->setBrush(QBrush(QColor(255, 0, 0, 150)));
            painter->drawPath(m_currentPath);
        }
    }

    virtual QRectF boundingRect() const override { return m_sourcePath.boundingRect().united(m_currentPath.boundingRect()); }

    //////////////////////////////////////////

    int type() const override { return m_type; }
    double sourceDrill() const { return m_sourceDrill; }
    QPolygonF toolPath() const { return m_toolPath; }

    double currentDrill() const { return m_currentDrill; }
    void setCurrentDrill(double currentDrill)
    {
        m_currentDrill = currentDrill;
        if (!qFuzzyIsNull(m_currentDrill)) {
            m_currentPath = QPainterPath();
            switch (m_type) {
            case Slot: {
                Paths tmpPpath;
                ClipperOffset offset;
                offset.AddPath(toPath(hole->state.path), jtRound, etOpenRound);
                offset.Execute(tmpPpath, m_currentDrill * 0.5 * uScale);
                for (Path& path : tmpPpath) {
                    path.append(path.first());
                    m_currentPath.addPolygon(toQPolygon(path));
                }
            } break;
            case Drill:
                m_currentPath.addEllipse(hole->state.offsetPos(), m_currentDrill * 0.5, m_currentDrill * 0.5);
                break;
            case Apetrure:
                m_currentPath.addEllipse(toQPointF(grob->state.curPos()), m_currentDrill * 0.5, m_currentDrill * 0.5);
                break;
            }
        }
        update();
    }

    void setSelected(bool value)
    {
        isSelected = value;
        if (isSelected) {
            m_pen.setColor(Qt::green);
            m_brush.setColor(Qt::green);
        } else {
            m_pen.setColor(Qt::darkGray);
            m_brush.setColor(Qt::darkGray);
        }
        update();
    }

    IntPoint pos() const
    {
        switch (m_type) {
        case Slot:
            return toIntPoint(hole->state.offsetPos());
        case Drill:
            return toIntPoint(hole->state.offsetPos());
        case Apetrure:
            return grob->state.curPos();
        }
        return IntPoint();
    }

    Paths paths() const
    {
        switch (m_type) {
        case Slot:
            return { toPath(hole->state.path) };
        case Drill:
            return hole->item->paths();
        case Apetrure: {
            return grob->paths;
        }
        }
        return Paths();
    }

    bool fit()
    {
        switch (m_type) {
        case Slot:
        case Drill:
            return m_sourceDrill > m_currentDrill;
        case Apetrure:
            return grob->gFile->apertures()->value(id)->fit(m_currentDrill);
        }
        return false;
    }

private:
    const int id = 0;
    const G::GraphicObject* const grob = nullptr;
    const Hole* const hole = nullptr;

    QPainterPath m_sourcePath;
    QPainterPath m_currentPath;

    const double m_sourceDrill;
    double m_currentDrill = 0.0;
    const QPolygonF m_toolPath;
    const Type m_type;

    QPen m_pen;
    QBrush m_brush;

    bool isSelected = false;
};

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

    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, &QTableView::customContextMenuRequested, this, &DrillForm::on_customContextMenuRequested);
    connect(ui->tableView, &QTableView::doubleClicked, this, &DrillForm::on_doubleClicked);
    connect(ui->tableView, &QTableView::clicked, this, &DrillForm::on_clicked);

    ui->tableView->setWordWrap(false);
    ui->pbCreate->setEnabled(false);
    ui->rb_drilling->setChecked(true);

    connect(ui->rb_drilling, &QRadioButton::toggled, [=] { worckType = ui->rb_drilling->isChecked() ? drilling : (ui->rb_profile->isChecked() ? profile : pocket); });
    connect(ui->rb_profile, &QRadioButton::toggled, [=] { worckType = ui->rb_drilling->isChecked() ? drilling : (ui->rb_profile->isChecked() ? profile : pocket); });
    connect(ui->rb_pocket, &QRadioButton::toggled, [=] { worckType = ui->rb_drilling->isChecked() ? drilling : (ui->rb_profile->isChecked() ? profile : pocket); });

    updateFiles();
    self = this;
}

DrillForm::~DrillForm()
{
    self = nullptr;
    clear();
    delete ui;
}

void DrillForm::setApertures(const QMap<int, QSharedPointer<G::AbstractAperture>>* value)
{
    m_type = tAperture;
    clear();
    m_apertures = *value;
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
                    PreviewItem* item = new PreviewItem(go, apertureIt.key());
                    m_sourcePreview[apertureIt.key()].append(QSharedPointer<PreviewItem>(item));
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

    QMap<int, double>::const_iterator toolIt;
    for (toolIt = m_tools.begin(); toolIt != m_tools.end(); ++toolIt) {
        QString name(QString("Tool Ø%1mm").arg(toolIt.value()));
        model->appendRow(name, drawDrillIcon(), toolIt.key());
        const DrillFile* file = static_cast<DrillFile*>(ui->cbxFile->currentData().value<void*>());
        bool isSlot = false;
        for (const Hole& hole : *file) {
            if (hole.state.tCode == toolIt.key()) {
                PreviewItem* item = nullptr;
                if (hole.state.path.isEmpty()) {
                    item = new PreviewItem(hole);
                } else {
                    item = new PreviewItem(hole, hole.state.path);
                    isSlot = true;
                }
                m_sourcePreview[toolIt.key()].append(QSharedPointer<PreviewItem>(item));
                Scene::self->addItem(item);
            }
        }
        model->setSlot(model->rowCount() - 1, isSlot);
        if (toolIt.value() != 0.0)
            pickUpTool(toolIt.key(), toolIt.value(), isSlot);
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
    disconnect(ui->cbxFile, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DrillForm::on_cbxFileCurrentIndexChanged);

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
        on_cbxFileCurrentIndexChanged(0);
    connect(ui->cbxFile, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DrillForm::on_cbxFileCurrentIndexChanged);
}

void DrillForm::on_pbClose_clicked()
{
    if (parent())
        static_cast<QWidget*>(parent())->close();
}

//static QDebug operator<<(QDebug debug, const IntPoint& p)
//{
//    //QDebugStateSaver saver(debug);
//    debug.nospace() << '(' << p.X << ", " << p.Y << ')';
//    return debug;
//}

void DrillForm::on_pbCreate_clicked()
{
    { //   drills only
        QMap<int, QPair<QPair<Path, Paths>, QVector<int>>> pathsMap;
        for (int row = 0; row < model->rowCount(); ++row) {
            int toolId = model->toolId(row);
            if (toolId != -1) {
                int apertureId = model->apertureId(row);
                pathsMap[toolId].second.append(apertureId);
                for (QSharedPointer<PreviewItem>& item : m_sourcePreview[apertureId]) {
                    if (item->type() == PreviewItem::Slot)
                        continue;
                    if (worckType == drilling && ToolHolder::tools[toolId].type != Tool::Engraving) {
                        pathsMap[toolId].first.first.append(item->pos());
                    } else if (ToolHolder::tools[toolId].type != Tool::Drill && item->fit()) {
                        pathsMap[toolId].first.second.append(item->paths());
                        //                        } else {
                        //                            pathsMap[toolId].first.first.append(item->pos());
                        //                        }
                    }
                }
            }
        }

        QMap<int, QPair<QPair<Path, Paths>, QVector<int>>>::iterator iterator;
        for (iterator = pathsMap.begin(); iterator != pathsMap.end(); ++iterator) {
            int toolId = iterator.key();
            QString indexes;
            QVector<int>& v = pathsMap[toolId].second;
            for (int id : v)
                indexes += QString::number(id) + (id != v.last() ? "," : "");

            if (!pathsMap[toolId].first.first.isEmpty()) {
                Path& path = pathsMap[toolId].first.first;
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
                GCodeFile* gcode = new GCodeFile({ path }, ToolHolder::tools[toolId], ui->dsbxDepth->value(), Drilling);
                gcode->setFileName(/*"Drill " +*/ ToolHolder::tools[toolId].name + (m_type ? " - T(" : " - D(") + indexes + ')');
                gcode->setSide(static_cast<AbstractFile*>(ui->cbxFile->currentData().value<void*>())->side());
                FileModel::self->addGcode(gcode);
            }
            if (!pathsMap[toolId].first.second.isEmpty()) {
                ReversePaths(pathsMap[toolId].first.second);
                ToolPathCreator tpc(pathsMap[toolId].first.second, true);

                GCodeFile* gcode = nullptr;
                switch (worckType) {
                case profile:
                    gcode = tpc.createProfile(ToolHolder::tools[toolId], ui->dsbxDepth->value(), Inner);
                    break;
                case pocket:
                    gcode = tpc.createPocket(ToolHolder::tools[toolId], ui->dsbxDepth->value(), false, 0, true);
                    break;
                default:
                    continue;
                    break;
                }
                if (!gcode)
                    continue;
                gcode->setFileName(/*"Slot Drill " +*/ ToolHolder::tools[toolId].name + " - T(" + indexes + ')');
                gcode->setSide(static_cast<AbstractFile*>(ui->cbxFile->currentData().value<void*>())->side());
                FileModel::self->addGcode(gcode);
            }
        }
    }
    { //   slots only
        QMap<int, QPair<Paths, QVector<int>>> pathsMap;
        for (int row = 0; row < model->rowCount(); ++row) {
            int selectedToolId = model->toolId(row);
            if (selectedToolId != -1) {
                int apertureId = model->apertureId(row);
                pathsMap[selectedToolId].second.append(apertureId);
                for (QSharedPointer<PreviewItem>& item : m_sourcePreview[apertureId]) {
                    if (item->type() == PreviewItem::Slot) {
                        if (item->sourceDrill() > item->currentDrill()) {
                            for (Path& path : offset(item->paths().first(), item->sourceDrill() - item->currentDrill())) {
                                path.append(path.first());
                                pathsMap[selectedToolId].first.append(path);
                            }
                        } else {
                            pathsMap[selectedToolId].first.append(item->paths().first());
                        }
                    }
                }
            }
        }
        QMap<int, QPair<Paths, QVector<int>>>::iterator iterator;
        for (iterator = pathsMap.begin(); iterator != pathsMap.end(); ++iterator) {
            int selectedToolId = iterator.key();
            QString indexes;
            QVector<int>& v = pathsMap[selectedToolId].second;
            for (int id : v)
                indexes += QString::number(id) + (id != v.last() ? "," : "");
            if (!pathsMap[selectedToolId].first.isEmpty()) {
                GCodeFile* gcode = new GCodeFile(pathsMap[selectedToolId].first, ToolHolder::tools[selectedToolId], ui->dsbxDepth->value(), Profile);
                gcode->setFileName(/*"Slot Drill " +*/ ToolHolder::tools[selectedToolId].name + " - T(" + indexes + ')');
                gcode->setSide(static_cast<AbstractFile*>(ui->cbxFile->currentData().value<void*>())->side());
                FileModel::self->addGcode(gcode);
            }
        }
    }
}

void DrillForm::on_cbxFileCurrentIndexChanged(int /*index*/)
{
    if (static_cast<AbstractFile*>(ui->cbxFile->currentData().value<void*>())->type() == FileType::Gerber)
        setApertures(static_cast<G::File*>(ui->cbxFile->currentData().value<void*>())->apertures());
    else
        setHoles(static_cast<DrillFile*>(ui->cbxFile->currentData().value<void*>())->tools());
}

void DrillForm::on_clicked(const QModelIndex& index)
{
    int apertureId = model->apertureId(index.row());
    for (QSharedPointer<PreviewItem>& item : m_sourcePreview[apertureId]) {
        item->setSelected(true);
    }
}

void DrillForm::on_doubleClicked(const QModelIndex& current)
{
    if (current.column() == 1) {
        ToolDatabase tdb(this, model->isSlot(current.row()) ? QVector<Tool::Type>{ Tool::EndMill } : (worckType ? QVector<Tool::Type>{ Tool::Drill, Tool::EndMill, Tool::Engraving } : QVector<Tool::Type>{ Tool::Drill, Tool::EndMill }));
        if (tdb.exec()) {
            int apertureId = model->apertureId(current.row());
            const Tool tool(tdb.tool());
            model->setToolId(current.row(), tool.id);
            createHoles(apertureId, tool.diameter);
            ui->pbCreate->setEnabled(true);
        }
    }
}

void DrillForm::on_currentChanged(const QModelIndex& current, const QModelIndex& previous)
{

    if (previous.isValid() && previous.row() != current.row()) {
        int apertureId = model->apertureId(previous.row());
        for (QSharedPointer<PreviewItem>& item : m_sourcePreview[apertureId]) {
            item->setSelected(false);
        }
    }

    if (0) {
        for (int row = 0; row < model->rowCount(); ++row) {
            int apertureId = model->apertureId(row);
            for (QSharedPointer<PreviewItem>& item : m_sourcePreview[apertureId]) {
                item->setSelected(false);
            }
        }

        const QModelIndexList selectedIndexes(ui->tableView->selectionModel()->selectedIndexes());

        for (const QModelIndex& index : selectedIndexes) {
            int apertureId = model->apertureId(index.row());
            for (QSharedPointer<PreviewItem>& item : m_sourcePreview[apertureId]) {
                item->setSelected(true);
            }
        }
    }

    if (previous.isValid() && previous.row() != current.row()) {
        int apertureId = model->apertureId(current.row());
        for (QSharedPointer<PreviewItem>& item : m_sourcePreview[apertureId]) {
            item->setSelected(true);
        }
    }
}

void DrillForm::on_customContextMenuRequested(const QPoint& pos)
{
    if (ui->tableView->selectionModel()->selectedIndexes().isEmpty())
        return;
    QMenu menu;
    menu.addAction(QIcon::fromTheme("view-form"), tr("&Select Tool"), [=] {
        ToolDatabase tdb(this, worckType ? QVector<Tool::Type>{ Tool::Drill, Tool::EndMill, Tool::Engraving } : QVector<Tool::Type>{ Tool::Drill, Tool::EndMill });
        if (tdb.exec()) {
            const Tool tool(tdb.tool());
            for (QModelIndex current : ui->tableView->selectionModel()->selectedIndexes()) {
                if (model->isSlot(current.row()) && tool.type == Tool::EndMill) {
                    model->setToolId(current.row(), tool.id);
                    createHoles(model->apertureId(current.row()), tool.diameter);
                    ui->pbCreate->setEnabled(true);
                } else if (!model->isSlot(current.row())) {
                    model->setToolId(current.row(), tool.id);
                    createHoles(model->apertureId(current.row()), tool.diameter);
                    ui->pbCreate->setEnabled(true);
                }
            }
        }
    });

    for (QModelIndex current : ui->tableView->selectionModel()->selectedIndexes()) {
        if (model->toolId(current.row()) != -1) {
            menu.addAction(QIcon::fromTheme("list-remove"), tr("&Remove Tool"), [=] {
                for (QModelIndex current : ui->tableView->selectionModel()->selectedIndexes()) {
                    model->setToolId(current.row(), -1);
                    createHoles(model->apertureId(current.row()), 0.0);
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
}

void DrillForm::createHoles(int toolId, double diameter)
{
    for (QSharedPointer<PreviewItem>& item : m_sourcePreview[toolId]) {
        item->setCurrentDrill(diameter);
    }
}

void DrillForm::pickUpTool(int apertureId, double diameter, bool isSlot)
{
    const double k = 0.05; // 5%
    const double drillDiameterMin = diameter * (1.0 - k);
    const double drillDiameterMax = diameter * (1.0 + k);
    QMap<int, Tool>::const_iterator toolIt;
    for (toolIt = ToolHolder::tools.begin(); !isSlot && toolIt != ToolHolder::tools.end(); ++toolIt) {
        if (toolIt.value().type == Tool::Drill && drillDiameterMin <= toolIt.value().diameter && drillDiameterMax >= toolIt.value().diameter) {
            model->setToolId(model->rowCount() - 1, toolIt.key());
            createHoles(apertureId, toolIt.value().diameter);
            for (QSharedPointer<PreviewItem>& item : m_sourcePreview[apertureId]) {
                item->setCurrentDrill(toolIt.value().diameter);
            }
            ui->pbCreate->setEnabled(true);
            return;
        }
    }
    for (toolIt = ToolHolder::tools.begin(); toolIt != ToolHolder::tools.end(); ++toolIt) {
        if (toolIt.value().type == Tool::EndMill && drillDiameterMin <= toolIt.value().diameter && drillDiameterMax >= toolIt.value().diameter) {
            model->setToolId(model->rowCount() - 1, toolIt.key());
            createHoles(apertureId, toolIt.value().diameter);
            for (QSharedPointer<PreviewItem>& item : m_sourcePreview[apertureId]) {
                item->setCurrentDrill(toolIt.value().diameter);
            }
            ui->pbCreate->setEnabled(true);
            return;
        }
    }
}

void DrillForm::clear()
{
    m_sourcePreview.clear();
}
