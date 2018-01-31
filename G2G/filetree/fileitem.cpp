#include "fileitem.h"

#include <toolpath/toolpathcreator.h>
#include <graphicsview/mygraphicsscene.h>
#include <QFileInfo>
#include <mainwindow.h>
QTimer FileItem::repaintTimer;
FileItem::FileItem(G::File* gerberFile)
    : gFile(gerberFile)
{
    gFile->gig->addToTheScene(MainWindow::getMainWindow()->getScene());
    connect(&repaintTimer, &QTimer::timeout, this, &FileItem::repaint);
    MainWindow::getMainWindow()->closeAllAct->setEnabled(true);
    repaintTimer.start(100);
}

FileItem::~FileItem()
{
    disconnect(&repaintTimer, &QTimer::timeout, this, &FileItem::repaint);
    if (MainWindow::getMainWindow()->isVisible())
        MainWindow::getMainWindow()->closeAllAct->setEnabled(AbstractItem::parent()->rowCount() > 1);
    delete gFile;
    MainWindow::getMainWindow()->getScene()->setSceneRect(0, 0, 0, 0);
    MainWindow::getMainWindow()->getScene()->update();
    //semaphore.tryAcquire();
    repaintTimer.start(100);
}

bool FileItem::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.column())
        switch (role) {
        //        case Qt::DisplayRole:
        //            return gerberFile->fileName;
        //            return true;
        case Qt::CheckStateRole:
            checkState = value.value<Qt::CheckState>();
            gFile->gig->setVisible(checkState == Qt::Checked);
            return true;
        default:
            break;
        }
    return false;
}

int FileItem::columnCount() const
{
    return 1;
}

int FileItem::rowCount() const
{
    return 0;
}

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
            QColor c = gFile->gig->brush().color();
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
    int count = parentItem->rowCount();
    int cc = (count > 1) ? (240.0 / (count - 1)) * row() : 0;
    QColor color(QColor::fromHsv(cc, 255 - cc * 0.2, 255, 150));
    gFile->gig->setBrush(color);
    MainWindow::getMainWindow()->getScene()->update();
    repaintTimer.stop();
}
