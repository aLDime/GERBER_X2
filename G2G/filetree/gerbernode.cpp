#include "gerbernode.h"
#include "filetree/fileholder.h"
#include <QFileInfo>
#include <mainwindow.h>

QTimer GerberNode::m_repaintTimer;
using namespace Gerber;

GerberNode::GerberNode(File* file)
    : m_id(FileHolder::addFile(file))
{
    FileHolder::file(m_id)->itemGroup()->addToTheScene();
    FileHolder::file(m_id)->itemGroup()->setZValue(-m_id);
    FileHolder::file<Gerber::File>(m_id)->rawItemGroup()->addToTheScene();
    FileHolder::file<Gerber::File>(m_id)->rawItemGroup()->setZValue(-m_id);
    //MainWindow::self->closeAllAct->setEnabled(true);
    connect(&m_repaintTimer, &QTimer::timeout, this, &GerberNode::repaint);
    m_repaintTimer.setSingleShot(true);
    m_repaintTimer.start(100);
}

GerberNode::~GerberNode()
{
    FileHolder::deleteFile(m_id);
    //MainWindow::self->closeAllAct->setEnabled(FileHolder::isEmpty());
    if (Scene::self) {
        Scene::self->setSceneRect(Scene::self->itemsBoundingRect());
        Scene::self->update();
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
            FileHolder::file(m_id)->itemGroup()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
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
            return FileHolder::file(m_id)->itemGroup()->isVisible() ? Qt::Checked : Qt::Unchecked;
        case Qt::DecorationRole: {
            QColor color(FileHolder::file(m_id)->color());
            color.setAlpha(255);
            QPixmap pixmap(16, 16);
            pixmap.fill(color);
            if (FileHolder::file<Gerber::File>(m_id)->itemsType() == Gerber::File::Raw) {
                QFont f;
                f.setBold(true);
                QPainter p(&pixmap);
                p.setFont(f);
                p.drawText(QRect(0, 0, 16, 16), Qt::AlignCenter, "R");
            }
            return pixmap;
        }
        case Qt::UserRole:
            return m_id;
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
        case Qt::UserRole:
            return m_id;
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
    FileHolder::file(m_id)->setColor(QColor::fromHsv(k, /* 255 - k * 0.2*/ 255, 255, 150));
    FileHolder::file(m_id)->itemGroup()->setBrush(FileHolder::file(m_id)->color());
    FileHolder::file<Gerber::File>(m_id)->rawItemGroup()->setPen(QPen(FileHolder::file(m_id)->color(), 0.0));
    Scene::self->update();
}
