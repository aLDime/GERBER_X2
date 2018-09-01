#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QMutex>
#include <parser.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QToolBar* zoomToolBar = addToolBar(tr("Zoom ToolBar"));
    zoomToolBar->setIconSize(QSize(22, 22));
    zoomToolBar->setObjectName(QStringLiteral("zoomToolBar"));
    zoomToolBar->setMovable(false);
    zoomToolBar->setFloatable(false);

    QAction* action = zoomToolBar->addAction(QIcon::fromTheme("zoom-fit-best"), tr("Zoom fit best"), ui->graphicsView, &MyGraphicsView::ZoomFit);
    action->setShortcut(QKeySequence::FullScreen);
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-original"), tr("Zoom original"), ui->graphicsView, &MyGraphicsView::Zoom100);
    action->setShortcut(tr("Ctrl+0"));
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-in"), tr("Zoom in"), ui->graphicsView, &MyGraphicsView::ZoomIn);
    action->setShortcut(QKeySequence::ZoomIn);
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-out"), tr("Zoom out"), ui->graphicsView, &MyGraphicsView::ZoomOut);
    action->setShortcut(QKeySequence::ZoomOut);

    connect(ui->actionOpen, &QAction::triggered, [=] {
        QStringList files(QFileDialog::getOpenFileNames(this, tr("Open File"), lastPath, tr("Images (*.gbr *.*)")));
        for (QString& fileName : files) {
            lastPath = fileName;
            openFile(fileName);
            break;
        }
    });

    G::Parser* gerberParser = new G::Parser;

    gerberParser->moveToThread(&gerberThread);
    connect(&gerberThread, &QThread::finished, gerberParser, &QObject::deleteLater);
    connect(this, &MainWindow::parseFile, gerberParser, &G::Parser::parseFile, Qt::QueuedConnection);
    connect(gerberParser, &G::Parser::fileReady, this, &MainWindow::fileReady);
    gerberThread.start(QThread::HighestPriority);

    connect(ui->graphicsView, &MyGraphicsView::FileDroped, this, &MainWindow::openFile);

    QSettings settings;
    lastPath = settings.value("lastPath").toString();
    openFile(lastPath);
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("lastPath", lastPath);

    gerberThread.quit();
    gerberThread.wait();
    delete ui;
}

void MainWindow::openFile(const QString& fileName)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr(""), tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }
    lastPath = QDir(fileName).path();

    emit parseFile(fileName);

    statusBar()->showMessage(tr("File loaded"), 2000);
}

void MainWindow::fileReady(G::File* file)
{
    qDebug("fileReady");
    if (m_file)
        delete m_file;
    m_file = file;
    m_file->itemGroup->addToTheScene(ui->graphicsView->scene());
}
