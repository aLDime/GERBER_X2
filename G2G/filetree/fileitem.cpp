#include "fileitem.h"

#include <QFileInfo>
#include <mainwindow.h>
#include <myscene.h>

QTimer FileItem::repaintTimer;

FileItem::FileItem(G::File* gerberFile)
    : gFile(gerberFile)
{
    gFile->itemGroup->addToTheScene(MyScene::self);
    connect(&repaintTimer, &QTimer::timeout, this, &FileItem::repaint);
    MainWindow::self->closeAllAct->setEnabled(true);
    repaintTimer.setSingleShot(true);
    repaintTimer.start(100);
}

FileItem::~FileItem()
{
    if (MainWindow::self->isVisible())
        MainWindow::self->closeAllAct->setEnabled(parentItem()->childCount());
    delete gFile;
    MyScene::self->setSceneRect(0, 0, 0, 0);
    MyScene::self->update();
    repaintTimer.start(100);
}

bool FileItem::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.column())
        switch (role) {
        case Qt::CheckStateRole:
            checkState = value.value<Qt::CheckState>();
            gFile->itemGroup->setVisible(checkState == Qt::Checked);
            return true;
        default:
            break;
        }
    return false;
}

int FileItem::columnCount() const { return 1; }

int FileItem::childCount() const { return 0; }

Qt::ItemFlags FileItem::flags(const QModelIndex& /*index*/) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
}

QVariant FileItem::data(const QModelIndex& index, int role) const
{
    if (!index.column())
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return QFileInfo(gFile->fileName).fileName();
        case Qt::EditRole:
            return gFile->fileName;
        case Qt::CheckStateRole:
            return checkState;
        case Qt::DecorationRole: {
            QPixmap p(16, 16);
            QColor c = gFile->itemGroup->brush().color();
            c.setAlpha(255);
            p.fill(c);
            return p;
        }
        case Qt::UserRole:
            return QVariant::fromValue(reinterpret_cast<quint64>(gFile));
        default:
            break;
        }
    return QVariant();
}

void FileItem::repaint()
{
    int count = m_parentItem->childCount();
    int cc = (count > 1) ? (240.0 / (count - 1)) * row() : 0;
    QColor color(QColor::fromHsv(cc, 255 - cc * 0.2, 255, 150));
    gFile->itemGroup->setBrush(color);
    MyScene::self->update();
}
