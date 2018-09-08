#include "drillform.h"
#include "ui_drillform.h"

#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QPainter>
#include <QStandardItemModel>
#include <file.h>
#include <myscene.h>

#include <filetree/gerberitem.h>

#include "tooldatabase/tooldatabase.h"

DrillForm* DrillForm::self = nullptr;
enum { Size = 20 };

QIcon draw(G::Aperture* aperture)
{
    QPainterPath painterPath;

    for (QPolygonF& polygon : PathsToQPolygons(aperture->draw(G::State())))
        painterPath.addPolygon(polygon);

    painterPath.addEllipse(QPointF(0, 0), aperture->drillDiameter() * 0.5, aperture->drillDiameter() * 0.5);

    const QRectF rect = painterPath.boundingRect();

    double kh = 0, kw = 0;

    qreal scale = (double)Size / qMax(rect.width(), rect.height());

    if (rect.width() > rect.height())
        kh = (Size - rect.height() * scale) / 2;
    else
        kw = (Size - rect.width() * scale) / 2;

    QPixmap pixmap(Size, Size);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.translate(-rect.left() * scale + kw, rect.bottom() * scale + kh);
    painter.scale(scale, scale);
    painter.drawPath(painterPath);
    QIcon icon(pixmap);
    return icon;
}

DrillForm::DrillForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::DrillForm)
{
    ui->setupUi(this);
    ui->tableView->setIconSize(QSize(Size, Size));
    ui->formLayout;
    connect(ui->tableView, &QTableView::doubleClicked, this, &DrillForm::on_doubleClicked);

    updateFiles();

    self = this;
}

DrillForm::~DrillForm()
{
    if (MyScene::self)
        clear();

    delete ui;
    self = nullptr;
}

void DrillForm::setApertures(const QMap<int, G::Aperture*>& value)
{
    clear();

    apertures = value;
    QStandardItemModel* model = new QStandardItemModel(this);
    QList<QStandardItem*> list;
    QMapIterator<int, G::Aperture*> i(apertures);
    while (i.hasNext()) {
        i.next();
        if (i.value()->isFlashed()) {
            QString name;
            list.clear();
            name = QString("D%1, %2 mm").arg(i.key()).arg(i.value()->size());
            list << new QStandardItem(draw(i.value()), name);
            list.last()->setFlags(Qt::ItemIsEnabled);
            list.last()->setData(QVariant::fromValue<int>(i.key()));
            list << new QStandardItem("Select Drill");
            list.last()->setFlags(Qt::ItemIsEnabled);
            //            list.last()->setData(QVariant::fromValue<double>(0.0));
            model->appendRow(list);
        }
    }

    model->setHorizontalHeaderLabels(QString("Aperture|Tool").split('|'));
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
    for (G::File* file : GerberItem::gFiles) {
        for (const G::GraphicObject& go : *file) {
            if (go.state.curDCode == G::D03) {
                ui->cbxFile->addItem(QFileInfo(file->fileName).fileName(), QVariant::fromValue(static_cast<void*>(file)));
                ui->cbxFile->setItemData(ui->cbxFile->count() - 1, file->itemGroup->brush().color(), Qt::BackgroundColorRole);
                ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QSize(0, Size), Qt::SizeHintRole);
                break;
            }
        }
    }

    //    setApertures(static_cast<G::File*>(ui->cbxFile->currentData().value<void*>())->apertures);
}

void DrillForm::on_cbxFile_currentIndexChanged(int /*index*/)
{
    if (ui->cbxFile->count())
        setApertures(static_cast<G::File*>(ui->cbxFile->currentData().value<void*>())->apertures);
}

void DrillForm::on_doubleClicked(const QModelIndex& current)
{
    if (current.column() == 1) {
        ToolDatabase tdb(this, { Tool::Drill });
        if (tdb.exec()) {
            QStandardItem* stdItem = static_cast<const QStandardItemModel*>(current.model())->itemFromIndex(current);
            stdItem->setData(tdb.tool().name, Qt::DisplayRole);
            stdItem->setData(QVariant::fromValue(tdb.tool()));
        }
    }
}

void DrillForm::on_currentChanged(const QModelIndex& current, const QModelIndex& /*previous*/)
{
    QStandardItem* stdItem = static_cast<const QStandardItemModel*>(current.model())->item(current.row(), 0);
    int d = stdItem->data(Qt::UserRole + 1).toInt();
    clear();
    const G::File* f = static_cast<G::File*>(ui->cbxFile->currentData().value<void*>());
    for (const G::GraphicObject& go : *f) {
        if (go.state.curDCode == G::D03 && go.state.curAperture == d /*&& go.state.imgPolarity == G::POSITIVE*/) {

            QPainterPath painterPath;

            for (QPolygonF& polygon : PathsToQPolygons(apertures[d]->draw(G::State())))
                painterPath.addPolygon(polygon);

            painterPath.addEllipse(QPointF(0, 0), apertures[d]->drillDiameter() * 0.5, apertures[d]->drillDiameter() * 0.5);

            QGraphicsPathItem* item = new QGraphicsPathItem(painterPath);
            item->setAcceptHoverEvents(true);
            item->setPen(QPen(Qt::white, 0.0));
            item->setBrush(Qt::white);
            item->setPos(QPointF(go.state.curPos.X * dScale, go.state.curPos.Y * dScale));
            gia.append(item);

            MyScene::self->addItem(item);
        }
    }

    //        if (current.column() == 1) {
    //            //static_cast<QStandardItemModel*>(current.model())->itemFromIndex(current);
    //            //            QMessageBox::information(this, "", "");
    //            ToolDatabase tdb(this, { Tool::Drill });
    //            if (tdb.exec()) {
    //                QStandardItem* stdItem = static_cast<const QStandardItemModel*>(current.model())->itemFromIndex(current);
    //                stdItem->setData(tdb.tool().name, Qt::DisplayRole);
    //                stdItem->setData(QVariant::fromValue(tdb.tool()));
    //                //                m_tools[0] = tdb.tool();
    //                //                ui->lblToolName->setText(m_tools[0].name);
    //            }
    //        }
}

void DrillForm::clear()
{
    qDeleteAll(gia);
    gia.clear();
}
