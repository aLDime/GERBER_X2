#include "mainwindow.h"
#include "drillforapertureform.h"
#include "forms/drillform.h"
#include "forms/materialsetupform.h"
#include "forms/pocketform.h"
#include "forms/profileform.h"
#include "mainwindow.h"
#include "settingsdialog.h"
#include "tooldatabase/tooldatabase.h"
#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QtWidgets>
#include <filetree/fileholder.h>
#include <filetree/gcodenode.h>
#include <filetree/gerbernode.h>
#include <limits>
#include <myscene.h>
#include <parser.h>
#include <ui_drillform.h>

//#include "qt_windows.h"
//#include "Psapi.h"

MainWindow* MainWindow::self = nullptr;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , gerberParser(new G::Parser)
{
    setupUi(this);
    setToolTipDuration(0);
    //    MyGraphicsView::self = graphicsView;

    new Shtift();
    new Shtift();
    new Shtift();
    new Shtift();

    m_zeroPoint = new Point(Point::Zero);
    m_homePoint = new Point(Point::Home);

    MyScene::self->addItem(m_zeroPoint);
    MyScene::self->addItem(m_homePoint);

    init();

    gerberParser->moveToThread(&gerberThread);
    connect(&gerberThread, &QThread::finished, gerberParser, &QObject::deleteLater);
    connect(gerberParser, &G::Parser::fileReady, FileModel::self, &FileModel::addGerberFile);
    connect(gerberParser, &G::Parser::fileReady, [=](G::File* file) { prependToRecentFiles(file->fileName()); });
    connect(gerberParser, &G::Parser::fileProgress, this, &MainWindow::fileProgress);
    connect(gerberParser, &G::Parser::fileError, this, &MainWindow::fileError);
    connect(this, &MainWindow::parseFile, gerberParser, &G::Parser::parseFile, Qt::QueuedConnection);
    gerberThread.start(QThread::HighestPriority);

    connect(graphicsView, &MyGraphicsView::FileDroped, this, &MainWindow::openFile);
    graphicsView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(graphicsView, &MyGraphicsView::customContextMenuRequested, [=](const QPoint& pos) {
        //        QGraphicsItem* item = ;
        if (dynamic_cast<Shtift*>(MyScene::self->itemAt(graphicsView->mapToScene(pos), graphicsView->transform())) /*item && item->type() == ShtiftType*/) {
            QMenu menu;
            menu.addAction(QIcon::fromTheme("roll"), tr("&Create path for Shtifts"), [=] {
                ToolDatabase tdb(this, { Tool::Drill });
                if (tdb.exec()) {
                    Tool tool(tdb.tool());
                    Path dst;

                    for (Shtift* item : Shtift::shtifts()) {
                        item->setFlag(QGraphicsItem::ItemIsMovable, false);
                        IntPoint point(item->pos().x() * uScale, item->pos().y() * uScale);
                        if (dst.contains(point))
                            continue;
                        dst.append(point);
                    }

                    QSettings settings;
                    double depth = QInputDialog::getDouble(this, "", "Set Depth", settings.value("Shtift/depth").toDouble(), 0, 100, 2);
                    if (depth == 0.0)
                        return;
                    settings.setValue("Shtift/depth", depth);

                    GCodeFile* gcode = new GCodeFile({ dst }, {}, tool, depth, Drilling);
                    gcode->setFileName("Shtift");
                    FileModel::self->addGcode(gcode);
                }
            });
            menu.exec(graphicsView->mapToGlobal(pos + QPoint(24, 24)));
        }
    });

    if (0) {
        QPainterPath painterPath;
        QFont font;
        font.setPointSizeF(10);
        painterPath.addText(QPointF(1, -23), font, "Gerber X2");
        painterPath.addText(QPointF(1, -12), font, "to");
        painterPath.addText(QPointF(1, -01), font, "G Code");
        QGraphicsPathItem* pathItem = new QGraphicsPathItem(painterPath);
        pathItem->setAcceptHoverEvents(true);
        pathItem->setBrush(QColor(255, 180, 120));
        pathItem->setPen(Qt::NoPen);
        QTransform tr = QTransform::fromScale(2, -2);
        pathItem->setTransform(tr);
        MyScene::self->addItem(pathItem);
    }

    QLatin1String styleSheet("QGroupBox, .QFrame {"
                             //"background-color: rgb(255,255,255);"
                             "border: 1px solid gray;"
                             "border-radius: 5px;"
                             "margin-top: 1ex; /* leave space at the top for the title */"
                             "}"
                             "QGroupBox::title {"
                             "subcontrol-origin: margin;"
                             "margin-top: -2ex;"
                             "subcontrol-position: top center; /* position at the top center */"
                             "padding: 0 6px;"
                             "}");

    setStyleSheet(styleSheet);

    self = this;

    readSettings();

    //    graphicsView->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    gerberThread.quit();
    gerberThread.wait();
    self = nullptr;
}

Point* MainWindow::zero() const
{
    if (self)
        return m_zeroPoint;
    return nullptr;
}

Point* MainWindow::home() const
{
    if (self)
        return m_homePoint;
    return nullptr;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (QMessageBox::question(this, "", "Вы действительно хотите выйти из программы?", "Да", "Да?") == 0) {
        writeSettings();
        closeFiles();
        event->accept();
    } else
        event->ignore();
}

void MainWindow::open()
{
    QStringList files(QFileDialog::getOpenFileNames(this, tr("Open File"), lastPath, tr("Files (*.gbr *.*)")));
    for (QString& fileName : files) {
        openFile(fileName);
    }
}

void MainWindow::closeFiles()
{
    dockWidget->hide();
    FileModel::self->closeAllFiles();
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About G2G"), tr("G2G"));
}

void MainWindow::init()
{
    setAttribute(Qt::WA_DeleteOnClose);
    isUntitled = true;
    createActions();
    createStatusBar();
}

void MainWindow::createActions()
{
    QAction* action = nullptr;

    //==================== fileMenu ====================
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->setObjectName(QStringLiteral("fileMenu"));

    fileToolBar = addToolBar(tr("File"));
    fileToolBar->setIconSize(QSize(24, 24));
    fileToolBar->setContextMenuPolicy(Qt::CustomContextMenu);
    fileToolBar->setMovable(false);
    connect(fileToolBar, &QToolBar::customContextMenuRequested,
        [=](const QPoint& pos) {
            QMenu menu(this);
            menu.addAction(tr("Icon size = 24"), [=]() { fileToolBar->setIconSize(QSize(24, 24)); });
            menu.addAction(tr("Icon size = 48"), [=]() { fileToolBar->setIconSize(QSize(48, 48)); });
            menu.addAction(tr("Icon size = 72"), [=]() { fileToolBar->setIconSize(QSize(72, 72)); });
            menu.exec(fileToolBar->mapToGlobal(pos));
        });
    fileToolBar->setObjectName(QStringLiteral("fileToolBar"));

    action = fileMenu->addAction(QIcon::fromTheme("document-open"), tr("&Open..."), this, &MainWindow::open);
    fileToolBar->addAction(action);
    action->setShortcuts(QKeySequence::Open);
    action->setStatusTip(tr("Open an existing file"));

    action = fileMenu->addAction(QIcon::fromTheme("acrobat"), tr("&Export PDF..."), MyScene::self, &MyScene::RenderPdf);
    fileToolBar->addAction(action);
    action->setShortcuts(QKeySequence::Save);
    action->setStatusTip(tr("Export to PDF file"));

    fileMenu->addSeparator();
    fileMenu->addSeparator();

    recentMenu = fileMenu->addMenu(tr("Recent..."));
    connect(recentMenu, &QMenu::aboutToShow, this, &MainWindow::updateRecentFileActions);
    recentFileSubMenuAct = recentMenu->menuAction();

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = recentMenu->addAction(QString(), this, &MainWindow::openRecentFile);
        recentFileActs[i]->setVisible(false);
    }
    recentFileActs[MaxRecentFiles] = recentMenu->addAction("Clear Recent Files", [=] {
        QSettings settings;
        writeRecentFiles({}, settings);
        updateRecentFileActions();
        setRecentFilesVisible(MainWindow::hasRecentFiles());
    });

    recentFileSeparator = fileMenu->addSeparator();

    setRecentFilesVisible(MainWindow::hasRecentFiles());

    action = fileMenu->addAction(QIcon::fromTheme("document-close"), tr("&Close all"), this, &MainWindow::closeFiles);
    fileToolBar->addAction(action);
    action->setShortcuts(QKeySequence::Close);
    action->setStatusTip(tr("Close all files"));
    action->setEnabled(false);
    closeAllAct = action;

    action = fileMenu->addAction(QIcon::fromTheme("application-exit"), tr("E&xit"), qApp, &QApplication::closeAllWindows);
    action->setShortcuts(QKeySequence::Quit);
    action->setStatusTip(tr("Exit the application"));

    //==================== serviceMenu ====================
    serviceMenu = menuBar()->addMenu(tr("&Service"));
    action = serviceMenu->addAction(QIcon::fromTheme("configure-shortcuts"), tr("&Settings"), this, &MainWindow::showSettingsDialog);
    action->setStatusTip(tr("Show the application's settings box"));

    //==================== helpMenu ====================
    helpMenu = menuBar()->addMenu(tr("&Help"));
    action = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    action->setStatusTip(tr("Show the application's About box"));

    action = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    action->setStatusTip(tr("Show the Qt library's About box"));

    //==================== zoomToolBar ====================
    zoomToolBar = addToolBar(tr("Zoom ToolBar"));
    zoomToolBar->setIconSize(QSize(22, 22));
    zoomToolBar->setObjectName(QStringLiteral("zoomToolBar"));
    zoomToolBar->setMovable(false);
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-fit-best"), tr("Zoom fit best"), [=]() { graphicsView->ZoomFit(); });
    action->setShortcut(QKeySequence::FullScreen);
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-original"), tr("Zoom original"), [=]() { graphicsView->Zoom100(); });
    action->setShortcut(tr("Ctrl+0"));
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-in"), tr("Zoom in"), [=]() { graphicsView->ZoomIn(); });
    action->setShortcut(QKeySequence::ZoomIn);
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-out"), tr("Zoom out"), [=]() { graphicsView->ZoomOut(); });
    action->setShortcut(QKeySequence::ZoomOut);

    //==================== Selection ====================
    QToolBar* s = addToolBar(tr("Selection"));
    s->setObjectName(QStringLiteral("s"));
    s->setMovable(false);
    action = s->addAction(QIcon::fromTheme("edit-select-all"), tr("Select all"), [=]() {
        for (QGraphicsItem* item : MyScene::self->items())
            if (item->isVisible())
                item->setSelected(true);
    });
    action->setShortcut(QKeySequence::SelectAll);

    //==================== toolpathToolBar ====================
    toolpathToolBar = addToolBar(tr("Toolpath"));
    toolpathToolBar->setIconSize(QSize(24, 24));
    toolpathToolBar->setObjectName(QStringLiteral("toolpathToolBar"));
    toolpathToolBar->setMovable(false);
    dockWidget = new QDockWidget(this);
    dockWidget->setObjectName(QStringLiteral("dwCreatePath"));
    addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    dockWidget->hide();
    connect(dockWidget, &QDockWidget::visibilityChanged, [&](bool visible) {
        if (!visible) {
            QTimer::singleShot(100, [=] {
                if (dockWidget->isHidden()) {
                    for (QAction* action : toolpathActionList)
                        action->setChecked(false);

                    if (dockWidget->widget() != nullptr)
                        dockWidget->widget()->deleteLater();
                }
            });
        }
    });

    toolpathActionList.append(toolpathToolBar->addAction(QIcon::fromTheme("object-to-path"),
        tr("Profile"), [=] {
            createDockWidget(new ProfileForm(), Profile);
        }));
    toolpathActionList.append(toolpathToolBar->addAction(QIcon::fromTheme("stroke-to-path"),
        tr("Pocket"), [=] {
            createDockWidget(new PocketForm(), Pocket);
        }));
    toolpathActionList.append(toolpathToolBar->addAction(QIcon::fromTheme("roll"),
        tr("Drilling"), [=] {
            createDockWidget(new DrillForm(), Drilling);
        }));
    toolpathActionList.append(toolpathToolBar->addAction(QIcon::fromTheme("node"),
        tr("Setup Material "), [=] {
            createDockWidget(new MaterialSetup(), Material);
        }));

    for (QAction* action : toolpathActionList)
        action->setCheckable(true);

    QTimer::singleShot(10, [=] { createDockWidget(new MaterialSetup(), Material); });

    toolpathToolBar->addAction(QIcon::fromTheme("view-form"),
        tr("Tool Base"), [=]() {
            ToolDatabase tdb(this, {});
            tdb.exec();
        });

    QToolBar* toolBar = addToolBar(tr("Cursor mode"));
    toolBar->setMovable(false);
    action = toolBar->addAction(QIcon::fromTheme("transform-crop"), "Select", [=] {
        MyGraphicsView::self->setDragMode(QGraphicsView::RubberBandDrag);
        MyGraphicsView::self->setInteractive(true);
    });
    action = toolBar->addAction(QIcon::fromTheme("transform-browse"), "Drag", [=] {
        MyGraphicsView::self->setDragMode(QGraphicsView::ScrollHandDrag);
        MyGraphicsView::self->setInteractive(false);
    });
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings()
{
    QSettings settings;
    if (isHidden()) {
        const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();

        restoreGeometry(geometry);
    }
    lastPath = settings.value("lastPath").toString();
    QString files = settings.value("files").toString();
    for (const QString& file : files.split('|', QString::SkipEmptyParts))
        openFile(file);
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("lastPath", lastPath);
    settings.setValue("files", FileHolder::fileNames());
}

void MainWindow::fileProgress(const QString& fileName, int max, int value)
{
    static QProgressDialog progress;
    if (!value) {
        progress.setCancelButton(nullptr);
        progress.setLabelText(fileName);
        progress.setMaximum(max);
        progress.setModal(true);
        progress.setValue(0);
        progress.show();
    } else if (max == value)
        progress.hide();
    else
        progress.setValue(value);
}

void MainWindow::fileError(const QString& fileName, const QString& error)
{
    QMessageBox::critical(this, fileName, error);
}

void MainWindow::openFile(const QString& fileName)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    if (FileHolder::fileNames().contains(fileName)) {
        QMessageBox::warning(this, "", tr("The document is open."));
        return;
    }

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr(""), tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }
    QFileInfo fi(fileName);
    lastPath = fi.absolutePath();

    if (fi.suffix().contains("drl", Qt::CaseInsensitive)) {
        DrillFile* dFile = DrillParser().parseFile(fileName);
        if (dFile) {
            FileModel::self->addDrlFile(dFile);
            prependToRecentFiles(dFile->fileName());
        }
    } else
        emit parseFile(fileName);
}

void MainWindow::setRecentFilesVisible(bool visible)
{
    recentFileSubMenuAct->setVisible(visible);
    recentFileSeparator->setVisible(visible);
}

QStringList MainWindow::readRecentFiles(QSettings& settings)
{
    QStringList result;
    const int count = settings.beginReadArray(recentFilesKey());
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        result.append(settings.value(fileKey()).toString());
    }
    settings.endArray();
    return result;
}

void MainWindow::writeRecentFiles(const QStringList& files, QSettings& settings)
{
    const int count = files.size();
    settings.beginWriteArray(recentFilesKey());
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        settings.setValue(fileKey(), files.at(i));
    }
    settings.endArray();
}

bool MainWindow::hasRecentFiles()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const int count = settings.beginReadArray(recentFilesKey());
    settings.endArray();
    return count > 0;
}

void MainWindow::prependToRecentFiles(const QString& fileName)
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QStringList oldRecentFiles = readRecentFiles(settings);
    QStringList recentFiles = oldRecentFiles;
    recentFiles.removeAll(fileName);
    recentFiles.prepend(fileName);
    if (oldRecentFiles != recentFiles)
        writeRecentFiles(recentFiles, settings);

    setRecentFilesVisible(!recentFiles.isEmpty());
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    const QStringList recentFiles = readRecentFiles(settings);
    const int count = qMin(int(MaxRecentFiles), recentFiles.size());
    int i = 0;
    for (; i < count; ++i) {
        const QString fileName = MainWindow::strippedName(recentFiles.at(i));
        recentFileActs[i]->setText(tr("&%1 %2").arg(i + 1).arg(fileName));
        recentFileActs[i]->setData(recentFiles.at(i));
        recentFileActs[i]->setVisible(true);
    }
    for (; i < MaxRecentFiles; ++i)
        recentFileActs[i]->setVisible(false);

    recentFileActs[MaxRecentFiles]->setVisible(count);
}

void MainWindow::openRecentFile()
{
    if (const QAction* action = qobject_cast<const QAction*>(sender()))
        openFile(action->data().toString());
}

void MainWindow::setCurrentFile(const QString& fileName)
{
    static int sequenceNumber = 1;

    isUntitled = fileName.isEmpty();

    if (isUntitled)
        curFile = tr("document%1.txt").arg(sequenceNumber++);
    else
        curFile = QFileInfo(fileName).canonicalFilePath();

    //    textEdit->document()->setModified(false);
    setWindowModified(false);

    if (!isUntitled && windowFilePath() != curFile)
        prependToRecentFiles(curFile);

    setWindowFilePath(curFile);
}

QString MainWindow::strippedName(const QString& fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::showSettingsDialog()
{
    SettingsDialog settings(this);
    if (settings.exec()) {
        //readSettings();
    }
}

void MainWindow::createDockWidget(QWidget* dwContent, int type)
{
    if (dockWidget->widget() != nullptr)
        delete dockWidget->widget();
    dwContent->setObjectName(QStringLiteral("dwContents"));
    dockWidget->setWidget(dwContent);
    //dwCreatePath->setFloating(false);
    dockWidget->setWindowTitle(tr("Create Toolpath"));
    for (QAction* action : toolpathActionList)
        action->setChecked(false);
    toolpathActionList[type]->setChecked(true);
    dockWidget->show();
}
