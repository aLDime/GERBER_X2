#include "gerbernode.h"

#include "staticholders/fileholder.h"
#include <QFileInfo>
#include <mainwindow.h>

QTimer GerberNode::m_repaintTimer;
using namespace G;

GerberNode::GerberNode(File* file)
    : m_id(FileHolder::addFile(file))
{
    FileHolder::file(m_id)->itemGroup()->addToTheScene();
    FileHolder::file(m_id)->itemGroup()->setZValue(-m_id);
    MainWindow::self->closeAllAct->setEnabled(true);
    connect(&m_repaintTimer, &QTimer::timeout, this, &GerberNode::repaint);
    m_repaintTimer.setSingleShot(true);
    m_repaintTimer.start(100);
    MyGraphicsView::self->zoomFit();
}

GerberNode::~GerberNode()
{
    FileHolder::deleteFile(m_id);
    MainWindow::self->closeAllAct->setEnabled(FileHolder::isEmpty());
    if (MyScene::self) {
        MyScene::self->setSceneRect(MyScene::self->itemsBoundingRect());
        MyScene::self->update();
    }
    disconnect(&m_repaintTimer, &QTimer::timeout, this, &GerberNode::repaint);
    m_repaintTimer.start(1);
}

bool GerberNode::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::CheckStateRole:
            checkState = value.value<Qt::CheckState>();
            FileHolder::file(m_id)->itemGroup()->setVisible(checkState == Qt::Checked);
            return true;
        default:
            return false;
        }
    case 1:
        switch (role) {
        case Qt::EditRole:
            FileHolder::file(m_id)->setSide(static_cast<Side>(value.toBool()));
            return true;

        default:
            return false;
        }
    default:
        return false;
    }
    return false;
}

Qt::ItemFlags GerberNode::flags(const QModelIndex& index) const
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

QVariant GerberNode::data(const QModelIndex& index, int role) const
{
    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::DisplayRole:
            return FileHolder::file(m_id)->shortFileName();
        case Qt::ToolTipRole:
            return FileHolder::file(m_id)->fileName();
        case Qt::CheckStateRole:
            return checkState;
        case Qt::DecorationRole: {
            QPixmap pixmap(16, 16);
            QColor color = FileHolder::file(m_id)->itemGroup()->brush().color();
            color.setAlpha(255);
            pixmap.fill(color);
            return pixmap;
        }
        case Qt::UserRole:
            return QVariant::fromValue(static_cast<void*>(FileHolder::file(m_id)));
        default:
            return QVariant();
        }
    case 1:
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return QString("Top|Bottom").split('|')[FileHolder::file(m_id)->side()];
        case Qt::EditRole:
            return static_cast<bool>(FileHolder::file(m_id)->side());
        default:
            return QVariant();
        }
    default:
        break;
    }
    return QVariant();
}

QTimer* GerberNode::repaintTimer()
{
    return &m_repaintTimer;
}

void GerberNode::repaint()
{
    int count = m_parentItem->childCount();
    int k = (count > 1) ? (200.0 / (count - 1)) * row() : 0;
    QColor color(QColor::fromHsv(k, /* 255 - k * 0.2*/ 255, 255, 150));
    FileHolder::file(m_id)->itemGroup()->setBrush(color);
    MyScene::self->update();
}
