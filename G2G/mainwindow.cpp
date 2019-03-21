#include "mainwindow.h"
#include "aboutform.h"
#include "settingsdialog.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QProgressDialog>
#include <QToolBar>

#include <parser.h>

#include <forms/drillform.h>
#include <forms/materialsetupform.h>
#include <forms/pocketform.h>
#include <forms/profileform.h>

#include <tooldatabase/tooldatabase.h>

#include <staticholders/fileholder.h>

#include <gi/bridgeitem.h>

MainWindow* MainWindow::self = nullptr;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)

    , gerberParser(new G::Parser)
    , m_zeroPoint(new Point(Point::Zero))
    , m_homePoint(new Point(Point::Home))
{
    setupUi(this);
    setToolTipDuration(0);

    new Shtift();
    new Shtift();
    new Shtift();
    new Shtift();

    MyScene::self->addItem(m_zeroPoint);
    MyScene::self->addItem(m_homePoint);

    init();

    gerberParser->moveToThread(&gerberThread);
    connect(this, &MainWindow::parseFile, gerberParser, &G::Parser::parseFile, Qt::QueuedConnection);
    connect(gerberParser, &G::Parser::fileReady, FileModel::self, &FileModel::addGerberFile);
    connect(gerberParser, &G::Parser::fileReady, [=](G::File* file) { prependToRecentFiles(file->fileName()); });
    connect(gerberParser, &G::Parser::fileProgress, this, &MainWindow::fileProgress);
    connect(gerberParser, &G::Parser::fileError, this, &MainWindow::fileError);
    connect(&gerberThread, &QThread::finished, gerberParser, &QObject::deleteLater);
    gerberThread.start(QThread::HighestPriority);

    connect(graphicsView, &MyGraphicsView::fileDroped, this, &MainWindow::openFile);
    graphicsView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(graphicsView, &MyGraphicsView::customContextMenuRequested, this, &MainWindow::on_customContextMenuRequested);

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
                             "padding: 1ex;"
                             "}"
                             "QGroupBox::title {"
                             "subcontrol-origin: margin;"
                             "margin-top: -2ex;"
                             "subcontrol-position: top center; /* position at the top center */"
                             "padding: 0 1ex;"
                             "}");

    setStyleSheet(styleSheet);

    self = this;

    ToolHolder::readTools();

    readSettings();
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
    event->accept();
    writeSettings();
    closeFiles();
    return;
    if (QMessageBox::question(this, "", "Do you really want to quit the program?", "Yes", "No") == 0) {
        writeSettings();
        closeFiles();
        //        disconnect(&gerberThread, &QThread::finished, gerberParser, &QObject::deleteLater);
        //        disconnect(gerberParser, &G::Parser::fileReady, FileModel::self, &FileModel::addGerberFile);
        //        disconnect(gerberParser, &G::Parser::fileProgress, this, &MainWindow::fileProgress);
        //        disconnect(gerberParser, &G::Parser::fileError, this, &MainWindow::fileError);
        //        disconnect(this, &MainWindow::parseFile, gerberParser, &G::Parser::parseFile);
        //        if (gerberThread.isRunning())
        //            gerberThread.terminate();
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
    dockWidget->close();
    FileModel::self->closeAllFiles();
}

void MainWindow::about()
{
    AboutForm a(this);
    a.exec();
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

    exportPdfAct = fileMenu->addAction(QIcon::fromTheme("acrobat"), tr("&Export PDF..."), MyScene::self, &MyScene::RenderPdf);
    fileToolBar->addAction(exportPdfAct);
    exportPdfAct->setShortcuts(QKeySequence::Save);
    exportPdfAct->setStatusTip(tr("Export to PDF file"));
    exportPdfAct->setEnabled(false);

    fileMenu->addSeparator();
    fileMenu->addSeparator();

    recentMenu = fileMenu->addMenu(tr("Recent..."));
    connect(recentMenu, &QMenu::aboutToShow, this, &MainWindow::updateRecentFileActions);
    recentFileSubMenuAct = recentMenu->menuAction();

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = recentMenu->addAction(QString(), this, &MainWindow::openRecentFile);
        recentFileActs[i]->setVisible(false);
    }
    recentMenu->addSeparator();
    recentFileActs[MaxRecentFiles] = recentMenu->addAction("Clear Recent Files", [=] {
        QSettings settings;
        writeRecentFiles({}, settings);
        updateRecentFileActions();
        setRecentFilesVisible(MainWindow::hasRecentFiles());
    });

    recentFileSeparator = fileMenu->addSeparator();

    setRecentFilesVisible(MainWindow::hasRecentFiles());

    closeAllAct = fileMenu->addAction(QIcon::fromTheme("document-close"), tr("&Close all"), this, &MainWindow::closeFiles);
    fileToolBar->addAction(closeAllAct);
    closeAllAct->setShortcuts(QKeySequence::Close);
    closeAllAct->setStatusTip(tr("Close all files"));
    closeAllAct->setEnabled(false);

    action = fileMenu->addAction(QIcon::fromTheme("application-exit"), tr("E&xit"), qApp, &QApplication::closeAllWindows);
    action->setShortcuts(QKeySequence::Quit);
    action->setStatusTip(tr("Exit the application"));

    //==================== serviceMenu ====================
    serviceMenu = menuBar()->addMenu(tr("&Service"));
    action = serviceMenu->addAction(QIcon::fromTheme("configure-shortcuts"), tr("&Settings"),
        this, &MainWindow::showSettingsDialog);
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
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-fit-best"), tr("Fit best"), [=]() { graphicsView->zoomFit(); });
    action->setShortcut(QKeySequence::FullScreen);
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-original"), tr("100%"), [=]() { graphicsView->zoom100(); });
    action->setShortcut(tr("Ctrl+0"));
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-in"), tr("Zoom in"), [=]() { graphicsView->zoomIn(); });
    action->setShortcut(QKeySequence::ZoomIn);
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-out"), tr("Zoom out"), [=]() { graphicsView->zoomOut(); });
    action->setShortcut(QKeySequence::ZoomOut);

    //==================== Selection / Delete selected ====================
    QToolBar* s = addToolBar(tr("Selection"));
    s->setObjectName(QStringLiteral("s"));
    s->setMovable(false);
    action = s->addAction(QIcon::fromTheme("edit-select-all"), tr("Select all"), [=]() {
        for (QGraphicsItem* item : MyScene::self->items())
            if (item->isVisible())
                item->setSelected(true);
    });
    action->setShortcut(QKeySequence::SelectAll);
    action = s->addAction(/*QIcon::fromTheme("edit-select-all"),*/ tr("Zoom to selected"), [=]() { graphicsView->zoomToSelected(); });
    //    action = s->addAction(QIcon::fromTheme("layer-delete"), tr("Delete selected"), [=]() {
    //        QList<QGraphicsItem*> list;
    //        for (QGraphicsItem* item : MyScene::self->items())
    //            if (item->isSelected() && item->type() != DrillItemType)
    //                list << item;
    //        if (list.size() && QMessageBox::question(this,
    //"", "Do you really want to delete the selected items?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
    //            for (QGraphicsItem* item : list)
    //                if (item->isSelected() && item->type() != DrillItemType)
    //                    delete item;
    //            MyScene::self->setSceneRect(MyScene::self->itemsBoundingRect());
    //            MyScene::self->update();
    //            MainWindow::self->zero()->resetPos();
    //            MainWindow::self->home()->resetPos();
    //            Shtift::shtifts()[0]->resetPos();
    //        }
    //    });
    //    action->setShortcut(QKeySequence::Delete);

    //==================== toolpathToolBar ====================
    toolpathToolBar = addToolBar(tr("Toolpath"));
    toolpathToolBar->setIconSize(QSize(24, 24));
    toolpathToolBar->setObjectName(QStringLiteral("toolpathToolBar"));
    toolpathToolBar->setMovable(false);
    dockWidget = new DockWidget(this);
    dockWidget->setObjectName(QStringLiteral("dwCreatePath"));
    addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    dockWidget->hide();
    //    connect(dockWidget, &QDockWidget::visibilityChanged, [&](bool visible) {
    //        if (!visible) {
    //            for (QAction* action : toolpathActionList)
    //                action->setChecked(false);
    //            if (dockWidget->widget() != nullptr)
    //                dockWidget->widget()->deleteLater();
    //            dockWidget->toggleViewAction()->setVisible(false);
    //        }
    //    });

    toolpathActionList.append(toolpathToolBar->addAction(QIcon::fromTheme("object-to-path"), tr("Profile"), [=] {
        createDockWidget(new ProfileForm(), Profile);
    }));
    toolpathActionList.append(toolpathToolBar->addAction(QIcon::fromTheme("stroke-to-path"), tr("Pocket"), [=] {
        createDockWidget(new PocketForm(), Pocket);
    }));
    toolpathActionList.append(toolpathToolBar->addAction(QIcon::fromTheme("roll"), tr("Drilling"), [=] {
        createDockWidget(new DrillForm(), Drilling);
    }));
    toolpathActionList.append(toolpathToolBar->addAction(QIcon::fromTheme("node"), tr("Setup Material "), [=] {
        createDockWidget(new MaterialSetup(), Material);
    }));

    for (QAction* action : toolpathActionList)
        action->setCheckable(true);

    QTimer::singleShot(10, [=] { toolpathActionList[Profile]->trigger(); });

    toolpathToolBar->addAction(QIcon::fromTheme("view-form"), tr("Tool Base"), [=] {
        ToolDatabase tdb(this, {});
        tdb.exec();
    });
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createShtifts()
{
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

        GCodeFile* gcode = new GCodeFile({ dst }, tool, depth, Drilling);
        gcode->setFileName("Shtift");
        FileModel::self->addGcode(gcode);
    }
}

void MainWindow::readSettings()
{
    QSettings settings;
    if (isHidden()) {
        const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
        restoreGeometry(geometry);

        const QByteArray state = settings.value("state", QByteArray()).toByteArray();
        restoreState(state);
    }
    lastPath = settings.value("lastPath").toString();
    QString files = settings.value("files").toString();
    for (const QString& file : files.split('|', QString::SkipEmptyParts))
        openFile(file);
    SettingsDialog().readSettings();

    //QTimer::singleShot(1000, Qt::CoarseTimer, [=] { SettingsDialog sd(this);  sd.exec(); graphicsView->update(); });
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("lastPath", lastPath);
    settings.setValue("files", FileHolder::fileNames());
}

void MainWindow::on_customContextMenuRequested(const QPoint& pos)
{
    QMenu menu;
    QAction* a = nullptr;
    QGraphicsItem* item = MyScene::self->itemAt(graphicsView->mapToScene(pos), graphicsView->transform());

    if (!item)
        return;

    if (item->type() == ShtiftType) {
        a = menu.addAction(QIcon::fromTheme("roll"), tr("&Create path for Shtifts"), this, &MainWindow::createShtifts);
        a = menu.addAction(tr("Fixed"));
        a->setCheckable(true);
        a->setChecked(!(Shtift::shtifts()[0]->flags() & QGraphicsItem::ItemIsMovable));
        connect(a, &QAction::toggled, [](bool fl) {
            for (Shtift* item : Shtift::shtifts())
                item->setFlag(QGraphicsItem::ItemIsMovable, !fl);
        });
    }
    if (dynamic_cast<Point*>(item)) {
        a = menu.addAction(tr("Fixed"));
        a->setCheckable(true);
        a->setChecked(!(item->flags() & QGraphicsItem::ItemIsMovable));
        connect(a, &QAction::toggled, [=](bool fl) { item->setFlag(QGraphicsItem::ItemIsMovable, !fl); });
    }
    if (a)
        menu.exec(graphicsView->mapToGlobal(pos + QPoint(24, 0)));
}

void MainWindow::fileProgress(const QString& fileName, int max, int value)
{
    static QMap<QString, QProgressDialog*> progress;
    if (max && !value) {
        QProgressDialog* pd = new QProgressDialog(this);
        pd->setCancelButton(nullptr);
        pd->setLabelText(fileName);
        pd->setMaximum(max);
        pd->setModal(true);
        pd->setWindowFlag(Qt::WindowCloseButtonHint, false);
        pd->show();
        progress[fileName] = pd;
    } else if (max == 1 && value == 1) {
        progress[fileName]->hide();
        progress[fileName]->deleteLater();
        progress.remove(fileName);
    } else
        progress[fileName]->setValue(value);
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
        QMessageBox::warning(this,
            tr(""),
            tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }
    QFileInfo fi(fileName);
    lastPath = fi.absolutePath();
    DrillParser dp;
    if (dp.isDrillFile(fileName)) {
        DrillFile* dFile = dp.parseFile(fileName);
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

    //textEdit->document()->setModified(false);
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
    dwContent->setObjectName(QStringLiteral("dwContents"));

    for (QAction* action : toolpathActionList)
        action->setChecked(false);

    toolpathActionList[type]->setChecked(true);

    if (dockWidget->widget())
        delete dockWidget->widget();

    dockWidget->setWidget(dwContent);
    dockWidget->setWindowTitle(tr("Create Toolpath"));
    dockWidget->show();
}
