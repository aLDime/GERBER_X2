#include "file.h"

#include <toolpathcreator.h>
#include <graphicsview/mygraphicsscene.h>
#include <QFileInfo>
#include <mainwindow.h>
QTimer File::repaintTimer;
File::File(GerberFile* gerberFile)
    : gerberFile(gerberFile)
    , checkState(Qt::Checked)
{
    MyGraphicsScene* scene = MainWindow::getMainWindow()->getScene();
    gig = new GerberItemGroup;
    ToolPathCreator tpc;
    tpc.Merge(gerberFile);
    int counter = 0;

    for (Paths& vpaths : tpc.GetGroupedPaths(COPPER)) {
        gig->append(new GerberWorkItem(vpaths));
        gig->last()->setToolTip(QString("COPPER %1").arg(++counter));
    }

    gig->addToTheScene(scene);
    repaintTimer.connect(&repaintTimer, &QTimer::timeout, this, &File::repaint);
    repaintTimer.start(100);
    MainWindow::getMainWindow()->closeAllAct->setEnabled(true);
}

File::~File()
{
    qDebug() << "~File()";
    if (MainWindow::getMainWindow()->isVisible())
        MainWindow::getMainWindow()->closeAllAct->setEnabled(AbstractItem::parent()->rowCount(QModelIndex()) > 1);
    delete gerberFile;
    delete gig;
    MainWindow::getMainWindow()->getScene()->setSceneRect(0, 0, 0, 0);
    MainWindow::getMainWindow()->getScene()->update();
    //semaphore.tryAcquire();
    repaintTimer.start(100);
}

bool File::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.column())
        switch (role) {
        //        case Qt::DisplayRole:
        //            return gerberFile->fileName;
        //            return true;
        case Qt::CheckStateRole:
            checkState = value.value<Qt::CheckState>();
            gig->setVisible(checkState == Qt::Checked);
            return true;
        default:
            break;
        }
    return false;
}

int File::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

int File::rowCount(const QModelIndex& /*parent*/) const
{
    return 0;
}

Qt::ItemFlags File::flags(const QModelIndex& /*index*/) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
}

QVariant File::data(const QModelIndex& index, int role) const
{
    if (!index.column())
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return QFileInfo(gerberFile->fileName).fileName();
        case Qt::EditRole:
            return gerberFile->fileName;
        case Qt::CheckStateRole:
            return checkState;
        case Qt::DecorationRole: {
            QPixmap p(16, 16);
            QColor c = gig->brush().color();
            c.setAlpha(255);
            p.fill(c);
            return p;
        }
        default:
            break;
        }
    return QVariant();
}

void File::repaint()
{
    int count = parentItem->rowCount(QModelIndex());
    int cc = (count > 1) ? (240.0 / (count - 1)) * row() : 0;
    QColor color(QColor::fromHsv(cc, 255 - cc * 0.2, 255, 150));
    gig->setBrush(color);
    MainWindow::getMainWindow()->getScene()->update();
    repaintTimer.stop();
}
