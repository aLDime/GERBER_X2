#include "drillitem.h"

#include <QFileInfo>
#include <mainwindow.h>
#include <myscene.h>

QMap<int, DrlFile*> DrillItem_::files;

DrillItem_::DrillItem_(DrlFile* file)
    : m_id(files.size() ? files.lastKey() + 1 : 0)
{
    files[m_id] = file;
    files[m_id]->itemGroup->addToTheScene(MyScene::self);
    files[m_id]->itemGroup->setZValue(-m_id);
    MyGraphicsView::self->ZoomFit();
    MyGraphicsView::self->Zoom100();
    MainWindow::self->closeAllAct->setEnabled(true);

    //    connect(&m_repaintTimer, &QTimer::timeout, this, &DrillItem_::repaint);
    //    m_repaintTimer.setSingleShot(true);
    //    m_repaintTimer.start(100);
}

DrillItem_::~DrillItem_()
{
    if (MainWindow::self && MainWindow::self->isVisible())
        MainWindow::self->closeAllAct->setEnabled(parentItem()->childCount());
    if (files.contains(m_id) && files[m_id])
        delete files.take(m_id);
    if (MyScene::self) {
        MyScene::self->setSceneRect(0, 0, 0, 0);
        MyScene::self->update();
    }
    //    m_repaintTimer.start(100);
}

bool DrillItem_::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::CheckStateRole:
            checkState = value.value<Qt::CheckState>();
            files[m_id]->itemGroup->setVisible(checkState == Qt::Checked);
            return true;
        default:
            return false;
        }

    default:
        return false;
    }
    return false;
}

int DrillItem_::columnCount() const { return 3; }

int DrillItem_::childCount() const { return 0; }

Qt::ItemFlags DrillItem_::flags(const QModelIndex& index) const
{
    switch (index.column()) {
    case 0:
        return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;

    default:
        return Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
    }
}

QVariant DrillItem_::data(const QModelIndex& index, int role) const
{
    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return QFileInfo(files[m_id]->fileName).fileName();
        case Qt::EditRole:
            return files[m_id]->fileName;
        case Qt::CheckStateRole:
            return checkState;
        case Qt::DecorationRole:
            return QIcon::fromTheme("roll");
        case Qt::UserRole:
            return QVariant::fromValue(static_cast<void*>(files[m_id]));
        default:
            return QVariant();
        }
    default:
        break;
    }

    return QVariant();
}

//QTimer* DrillItem_::repaintTimer()
//{
//    return &m_repaintTimer;
//}

//void DrillItem_::repaint()
//{
//    int count = m_parentItem->childCount();
//    int k = (count > 1) ? (240.0 / (count - 1)) * row() : 0;
//    QColor color(QColor::fromHsv(k, /* 255 - k * 0.2*/ 255, 255, 150));
//    files[m_id]->itemGroup->setBrush(color);
//    MyScene::self->update();
//}
