#include "drillforapertureform.h"
#include "gerberfileholder.h"

#include <QBoxLayout>
#include <QDebug>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QTableView>
#include <gerber/gerber.h>
#include <graphicsview/mygraphicsscene.h>
#include <graphicsview/mygraphicsview.h>

DrillForApertureForm::DrillForApertureForm(G::File* file, QWidget* parent)
    : QDialog(parent)
    , fileName(file->fileName)
    , apertures(file->apertures)
{
    setupUi(this);

    auto draw = [](G::Aperture* aperture) {
        QPainterPath painterPath;
        for (QPolygonF& polygon : PathsToQPolygons(aperture->draw(G::State()))) {
            painterPath.addPolygon(polygon);
        }
        painterPath.addEllipse(QPointF(0, 0), aperture->drillDiameter() * 0.5, aperture->drillDiameter() * 0.5);

        QRectF rect = painterPath.boundingRect();
        double width, height;
        int cx = 32;
        if (qFuzzyCompare(rect.width(), rect.height())) {
            width = cx;
            height = cx;
        }
        else if (rect.width() > rect.height()) {
            width = cx;
            height = rect.height() * ((double)cx / rect.width());
        }
        else {
            width = rect.width() * ((double)cx / rect.height());
            height = cx;
        }

        qreal scale = (double)cx / qMax(rect.width(), rect.height());
        QPixmap pixmap(ceil(width), ceil(height));
        pixmap.fill(Qt::transparent);

        QPainter painter;
        painter.begin(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);
        painter.translate(-painterPath.boundingRect().left() * scale, painterPath.boundingRect().bottom() * scale);
        painter.scale(scale, -scale);
        painter.drawPath(painterPath);
        QIcon icon(pixmap);
        return icon;
    };

    QStandardItemModel* model = new QStandardItemModel(this);
    QList<QStandardItem*> list;
    QMapIterator<int, G::Aperture*> i(apertures);
    while (i.hasNext()) {
        i.next();
        qDebug() << i.key();
        if (i.value()->isFlashed()) {
            QString name;
            list.clear();
            name = QString("D%1 %2").arg(i.key()).arg(i.value()->name());
            list << new QStandardItem(draw(i.value()), name);
            list.last()->setFlags(Qt::ItemIsEnabled);
            list.last()->setData(QVariant::fromValue<int>(i.key()));
            list << new QStandardItem("Select Drill");
            list.last()->setFlags(Qt::ItemIsEnabled);
            list.last()->setData(QVariant::fromValue<double>(0.0));
            model->appendRow(list);
        }
    }
    if (list.size() == 0) {
        QMessageBox::information(this, "", "No flashed apertures!");
        return;
    }
    model->setHorizontalHeaderLabels(QString("Gerber::G::Aperture|Tool").split('|'));

    tableView->setModel(model);
    tableView->resizeColumnsToContents();

    connect(tableView->selectionModel(), &QItemSelectionModel::currentChanged, [=](const QModelIndex& current, const QModelIndex& previous) {
        Q_UNUSED(previous)
        QStandardItem* stdItem = static_cast<QStandardItem*>(static_cast<const QStandardItemModel*>(current.model())->item(current.row(), 0));
        int d = stdItem->data(Qt::UserRole + 1).toInt();
        graphicsView->scene()->clear();
        QPainterPath painterPath;
        for (QPolygonF& polygon : PathsToQPolygons(apertures[d]->draw(G::State()))) {
            painterPath.addPolygon(polygon);
        }
        painterPath.addEllipse(QPointF(0, 0), apertures[d]->drillDiameter() * 0.5, apertures[d]->drillDiameter() * 0.5);

        QGraphicsPathItem* item = new QGraphicsPathItem(painterPath);
        item->setAcceptHoverEvents(true);
        item->setPen(Qt::NoPen);
        item->setBrush(Qt::gray);
        graphicsView->scene()->addItem(item);
        graphicsView->scene()->setSceneRect(graphicsView->scene()->itemsBoundingRect());
        graphicsView->ZoomFit();
        graphicsView->scale(0.95, 0.95);
        if (current.column() == 1) {
            //static_cast<QStandardItemModel*>(current.model())->itemFromIndex(current);
            QMessageBox::information(this, "", "");
        }
    });
}

void DrillForApertureForm::setupUi(QDialog* Dialog)
{
    if (Dialog->objectName().isEmpty())
        Dialog->setObjectName(QStringLiteral("Dialog"));

    Dialog->resize(728, 453);
    QVBoxLayout* verticalLayout = new QVBoxLayout(Dialog);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    QHBoxLayout* horizontalLayout = new QHBoxLayout();
    horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
    tableView = new QTableView(Dialog);
    tableView->setObjectName(QStringLiteral("tableView"));

    horizontalLayout->addWidget(tableView);

    graphicsView = new MyGraphicsView(Dialog);
    graphicsView->setObjectName(QStringLiteral("graphicsView"));
    graphicsView->setScene(new MyGraphicsScene(this));
    horizontalLayout->addWidget(graphicsView);

    verticalLayout->addLayout(horizontalLayout);

    buttonBox = new QDialogButtonBox(Dialog);
    buttonBox->setObjectName(QStringLiteral("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

    verticalLayout->addWidget(buttonBox);

    retranslateUi(Dialog);
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, this, &DrillForApertureForm::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, this, &DrillForApertureForm::reject);

    QMetaObject::connectSlotsByName(Dialog);
}

void DrillForApertureForm::retranslateUi(QDialog* Dialog)
{
    Dialog->setWindowTitle(tr("DrillForGerber::ApertureForm"));
}

void DrillForApertureForm::resizeEvent(QResizeEvent* event)
{
    graphicsView->ZoomFit();
    graphicsView->scale(0.95, 0.95);
    QDialog::resizeEvent(event);
}
