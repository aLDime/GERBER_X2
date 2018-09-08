#include "gerberitem.h"

#include <QFileInfo>
#include <mainwindow.h>
#include <myscene.h>

QTimer GerberItem::m_repaintTimer;
QMap<int, G::File*> GerberItem::gFiles;

GerberItem::GerberItem(G::File* file)
    : m_id(gFiles.size() ? gFiles.lastKey() + 1 : 0)
{
    gFiles[m_id] = file;
    gFiles[m_id]->itemGroup->addToTheScene(MyScene::self);
    gFiles[m_id]->itemGroup->setZValue(-m_id);
    MyGraphicsView::self->ZoomFit();
    MyGraphicsView::self->Zoom100();
    MainWindow::self->closeAllAct->setEnabled(true);

    connect(&m_repaintTimer, &QTimer::timeout, this, &GerberItem::repaint);
    m_repaintTimer.setSingleShot(true);
    m_repaintTimer.start(100);
}

GerberItem::~GerberItem()
{
    if (MainWindow::self && MainWindow::self->isVisible())
        MainWindow::self->closeAllAct->setEnabled(parentItem()->childCount());
    if (gFiles.contains(m_id) && gFiles[m_id])
        delete gFiles.take(m_id);
    if (MyScene::self) {
        MyScene::self->setSceneRect(0, 0, 0, 0);
        MyScene::self->update();
    }
    m_repaintTimer.start(100);
}

bool GerberItem::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::CheckStateRole:
            checkState = value.value<Qt::CheckState>();
            gFiles[m_id]->itemGroup->setVisible(checkState == Qt::Checked);
            return true;
        default:
            return false;
        }
    case 1:
        switch (role) {
        case Qt::EditRole:
            gFiles[m_id]->side = static_cast<G::Side>(value.toBool());
            return true;
        default:
            return false;
        }
    default:
        return false;
    }
    return false;
}

int GerberItem::columnCount() const { return 3; }

int GerberItem::childCount() const { return 0; }

Qt::ItemFlags GerberItem::flags(const QModelIndex& index) const
{
    switch (index.column()) {
    case 0:
        return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
    case 1:
        return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemNeverHasChildren;
    default:
        return Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
    }
}

QVariant GerberItem::data(const QModelIndex& index, int role) const
{
    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return QFileInfo(gFiles[m_id]->fileName).fileName();
        case Qt::EditRole:
            return gFiles[m_id]->fileName;
        case Qt::CheckStateRole:
            return checkState;
        case Qt::DecorationRole: {
            QPixmap pixmap(16, 16);
            QColor color = gFiles[m_id]->itemGroup->brush().color();
            color.setAlpha(255);
            pixmap.fill(color);
            return pixmap;
        }
        case Qt::UserRole:
            return QVariant::fromValue(static_cast<void*>(gFiles[m_id]));
        default:
            return QVariant();
        }
    case 1:
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return QString("Top|Bottom").split('|')[gFiles[m_id]->side];
        case Qt::EditRole:
            return static_cast<bool>(gFiles[m_id]->side);
        default:
            return QVariant();
        }
    default:
        break;
    }

    return QVariant();
}

QTimer* GerberItem::repaintTimer()
{
    return &m_repaintTimer;
}

void GerberItem::repaint()
{
    int count = m_parentItem->childCount();
    int k = (count > 1) ? (240.0 / (count - 1)) * row() : 0;
    QColor color(QColor::fromHsv(k, /* 255 - k * 0.2*/ 255, 255, 150));
    gFiles[m_id]->itemGroup->setBrush(color);
    MyScene::self->update();
}
