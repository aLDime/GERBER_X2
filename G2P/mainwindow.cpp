#include "mainwindow.h"
#include "../G2G/graphicsview/mygraphicsscene.h"
#include "../G2G/gerber/gerberparser.h"
#include <QDebug>
#include <QMenu>
#include <QThread>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    //    GerberParser* gerberParser = new GerberParser(this);
    //    QThread* gerberThread = new QThread(this);

    //    gerberParser->moveToThread(gerberThread);
    //    connect(gerberThread, &QThread::finished, gerberParser, &QObject::deleteLater);
    //    connect(this, &MainWindow::ParseFile, gerberParser, &GerberParser::ParseFile, Qt::QueuedConnection);
    //    connect(gerberParser, &GerberParser::resultReady, fileHolder, &GerberFileHolder::handleResults);
    //    gerberThread->start(QThread::HighestPriority);

    //    connect(graphicsView, &MyGraphicsView::FileDroped, [=]() {
    //        graphicsView->scene()->deleteLater();
    //        graphicsView->SetScene(new MyGraphicsScene(this));
    //    });

    //    setupUi(this);
    graphicsView->SetScene(new MyGraphicsScene(this));
    QAction* action;
    action = new QAction(tr("&Open File"), this);
    action->setShortcut(QKeySequence::Open);
    graphicsView->addAction(action);

    action = new QAction(tr("&Export PDF"), this);
    action->setShortcut(QKeySequence::Save);
    graphicsView->addAction(action);

    action = new QAction(tr("&Close"), this);
    action->setShortcut(QKeySequence::Close);
    connect(action, &QAction::triggered, [=]() {
        graphicsView->scene()->deleteLater();
        graphicsView->SetScene(new MyGraphicsScene(this));
    });
    graphicsView->addAction(action);

    action = new QAction(tr("&Exit"), this);
    action->setShortcut(QKeySequence::Quit);
    connect(action, &QAction::triggered, [=]() { close(); });
    graphicsView->addAction(action);

    graphicsView->setContextMenuPolicy(Qt::ActionsContextMenu);
}
