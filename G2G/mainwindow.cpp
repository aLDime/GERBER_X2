#include "mainwindow.h"
#include "aboutform.h"
#include "forms/drillform.h"
#include "forms/gcodepropertiesform.h"
#include "forms/pocketform.h"
#include "forms/profileform.h"
#include "forms/thermalform.h"
#include "forms/voronoiform.h"
#include "gi/bridgeitem.h"
#include "project.h"
#include "settingsdialog.h"
#include "tooldatabase/tooldatabase.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QProgressDialog>
#include <QTableView>
#include <QToolBar>
#include <excellondialog.h>
#include <exparser.h>
#include <filetree/gerbernode.h>
#include <gbrparser.h>

MainWindow* MainWindow::self = nullptr;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , gerberParser(new Gerber::Parser)
    , pro(new Project)
{
    setupUi(this);
    init();

    GCodePropertiesForm::homePoint = new Point(Point::Home);
    GCodePropertiesForm::zeroPoint = new Point(Point::Zero);

    new Shtift();
    new Shtift();
    new Shtift();
    new Shtift();

    Scene::addItem(GCodePropertiesForm::homePoint);
    Scene::addItem(GCodePropertiesForm::zeroPoint);

    gerberParser->moveToThread(&gerberThread);
    connect(this, &MainWindow::parseFile, gerberParser, &Gerber::Parser::parseFile, Qt::QueuedConnection);
    connect(gerberParser, &Gerber::Parser::fileReady, pro, &Project::addFile);
    connect(gerberParser, &Gerber::Parser::fileReady, [=](Gerber::File* file) { prependToRecentFiles(file->name()); });
    connect(gerberParser, &Gerber::Parser::fileReady, GerberNode::repaintTimer(), QOverload<>::of(&QTimer::start));
    connect(gerberParser, &Gerber::Parser::fileProgress, this, &MainWindow::fileProgress);
    connect(gerberParser, &Gerber::Parser::fileError, this, &MainWindow::fileError);
    connect(&gerberThread, &QThread::finished, gerberParser, &QObject::deleteLater);
    gerberThread.start(QThread::HighestPriority);

    connect(graphicsView, &GraphicsView::fileDroped, this, &MainWindow::loadFile);
    connect(graphicsView, &GraphicsView::customContextMenuRequested, this, &MainWindow::onCustomContextMenuRequested);

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
        Scene::addItem(pathItem);
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

    ToolHolder::readTools();
    setCurrentFile(QString());
    readSettings();

    self = this;
}

MainWindow::~MainWindow()
{
    gerberThread.quit();
    gerberThread.wait();
    self = nullptr;
}

//QAction* MainWindow::closeAllAct() { return self ? self->m_closeAllAct : nullptr; }
//QAction* MainWindow::exportPdfAct() { return self ? self->m_exportPdfAct : nullptr; }

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (maybeSave()) {
        writeSettings();
        dockWidget->close();
        FileModel::closeProject();
        qApp->closeAllWindows();
        event->accept();
    } else {
        event->ignore();
    }
    //    if (QFile("ui_mainwindow.h").exists()) {
    //        writeSettings();
    //        dockWidget->close();
    //        FileModel::closeProject();
    //        qApp->closeAllWindows();
    //        event->accept();
    //        return;
    //    }
    //    if (!Project::size() || QMessageBox::question(this, "", tr("Do you really want to quit the program?"), tr("No"), tr("Yes")) == 1) {
    //        qApp->closeAllWindows();
    //        writeSettings();
    //        dockWidget->close();
    //        FileModel::closeProject();
    //        event->accept();
    //    } else
    //        event->ignore();
}

void MainWindow::saveSelectedToolpaths()
{
    bool isEmpty = true;
    for (GCode::File* file : Project::files<GCode::File>()) {
        if (!file->itemGroup()->isVisible())
            continue;
        isEmpty = false;
        QString name(QFileDialog::getSaveFileName(this, tr("Save GCode file"),
            GCode::File::getLastDir().append(file->shortName()) + QStringList({ "(Top)", "(Bot)" })[file->side()],
            tr("GCode (*.tap)")));
        if (name.isEmpty())
            return;
        file->write(name);
        file->itemGroup()->setVisible(false);
    }
    if (isEmpty) {
        QMessageBox::information(this, "", tr("No selected toolpath files."));
    }
}

void MainWindow::closeProject()
{
    if (maybeSave()) {
        dockWidget->close();
        FileModel::closeProject();
        setCurrentFile(QString());
    }
    //    if (QFile("ui_mainwindow.h").exists()) {
    //        dockWidget->close();
    //        FileModel::closeProject();
    //        Project::setIsModified(false);
    //        return;
    //    }
    //    if (!Project::size() || QMessageBox::question(this, "", tr("Do you really want to close all files?"), tr("No"), tr("Yes")) == 1) {
    //        dockWidget->close();
    //        FileModel::closeProject();
    //        Project::setIsModified(false);
    //        setCurrentFile(QString());
    //    }
}

void MainWindow::about()
{
    AboutForm a(this);
    a.exec();
}

void MainWindow::init()
{
    isUntitled = true;
    createActions();
    connect(pro, &Project::changed, this, &MainWindow::documentWasModified);
    setUnifiedTitleAndToolBarOnMac(true);
}

void MainWindow::createActions()
{
    // fileMenu
    createActionsFile();
    // serviceMenu
    createActionsService();
    // helpMenu
    createActionsHelp();
    // zoomToolBar
    createActionsZoom();
    // Selection / Delete selected
    createActionsSDS();
    // toolpathToolBar
    createActionsToolPath();
    // grafica
    createActionsGraphics();

    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createActionsFile()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->setObjectName(QStringLiteral("fileMenu"));

    fileToolBar = addToolBar(tr("File"));
    fileToolBar->setContextMenuPolicy(Qt::CustomContextMenu);
    fileToolBar->setObjectName(QStringLiteral("fileToolBar"));
    // New
    QAction* action = fileMenu->addAction(/*Icon(OpenFileIcon), */ tr("&New project"), this, &MainWindow::newFile);
    action->setShortcuts(QKeySequence::New);
    action->setStatusTip(tr("Create a new file"));
    fileToolBar->addAction(action);
    // Open
    action = fileMenu->addAction(Icon(OpenFileIcon), tr("&Open..."), this, &MainWindow::open);
    action->setShortcuts(QKeySequence::Open);
    action->setStatusTip(tr("Open an existing file"));
    fileToolBar->addAction(action);
    // Save
    action = fileMenu->addAction(tr("&Save"), this, &MainWindow::save);
    action->setShortcuts(QKeySequence::Save);
    action->setStatusTip(tr("Save the document to disk"));
    fileToolBar->addAction(action);
    // Save As
    action = fileMenu->addAction(tr("Save &As..."), this, &MainWindow::saveAs);
    action->setShortcuts(QKeySequence::SaveAs);
    action->setStatusTip(tr("Save the document under a new name"));
    fileToolBar->addAction(action);
    // Save Selected Tool Paths
    action = fileMenu->addAction(Icon(SaveAllIcon), tr("&Save Selected Tool Paths..."), this, &MainWindow::saveSelectedToolpaths);
    action->setStatusTip(tr("Save selected toolpaths"));
    fileToolBar->addAction(action);
    // Export PDF
    action = fileMenu->addAction(Icon(SavePdfIcon), tr("&Export PDF..."), Scene::self, &Scene::RenderPdf);
    action->setStatusTip(tr("Export to PDF file"));
    fileToolBar->addAction(action);

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
    recentFileActs[MaxRecentFiles] = recentMenu->addAction(tr("Clear Recent Files"), [=] {
        QSettings settings;
        writeRecentFiles({}, settings);
        updateRecentFileActions();
        setRecentFilesVisible(MainWindow::hasRecentFiles());
    });

    recentFileSeparator = fileMenu->addSeparator();
    setRecentFilesVisible(MainWindow::hasRecentFiles());

    m_closeAllAct = fileMenu->addAction(Icon(CloseIcon), tr("&Close project \"%1\""), this, &MainWindow::closeProject);
    m_closeAllAct->setShortcuts(QKeySequence::Close);
    m_closeAllAct->setStatusTip(tr("Close project"));
    //    m_closeAllAct->setEnabled(false);
    fileToolBar->addAction(m_closeAllAct);

    fileMenu->addSeparator();
    action = fileMenu->addAction(/*Icon(ExitIcon), */ tr("P&rint"), this, &MainWindow::printDialog);
    action->setShortcuts(QKeySequence::Print);
    action->setStatusTip(tr("Print"));
    fileMenu->addSeparator();

    action = fileMenu->addAction(Icon(ExitIcon), tr("E&xit"), qApp, &QApplication::closeAllWindows);
    action->setShortcuts(QKeySequence::Quit);
    action->setStatusTip(tr("Exit the application"));
}

void MainWindow::createActionsService()
{
    serviceMenu = menuBar()->addMenu(tr("&Service"));
    QAction* action = serviceMenu->addAction(Icon(SettingsIcon), tr("&Settings"), [=] { SettingsDialog(this).exec(); });
    action->setStatusTip(tr("Show the application's settings box"));
}

void MainWindow::createActionsHelp()
{
    helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction* action = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    action->setStatusTip(tr("Show the application's About box"));

    action = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    action->setStatusTip(tr("Show the Qt library's About box"));
}

void MainWindow::createActionsZoom()
{
    zoomToolBar = addToolBar(tr("Zoom ToolBar"));
    //zoomToolBar->setIconSize(QSize(22, 22));
    zoomToolBar->setObjectName(QStringLiteral("zoomToolBar"));
    // zoomToolBar->setMovable(false);
    QAction* action = zoomToolBar->addAction(Icon(ZoomFitIcon), tr("Fit best"), [=]() { graphicsView->zoomFit(); });
    action->setShortcut(QKeySequence::FullScreen);
    action = zoomToolBar->addAction(Icon(Zoom100Icon), tr("100%"), [=]() { graphicsView->zoom100(); });
    action->setShortcut(tr("Ctrl+0"));
    action = zoomToolBar->addAction(Icon(ZoomInIcon), tr("Zoom in"), [=]() { graphicsView->zoomIn(); });
    action->setShortcut(QKeySequence::ZoomIn);
    action = zoomToolBar->addAction(Icon(ZoomOutIcon), tr("Zoom out"), [=]() { graphicsView->zoomOut(); });
    action->setShortcut(QKeySequence::ZoomOut);
    zoomToolBar->addSeparator();
    action = zoomToolBar->addAction(Icon(ZoomToSelectedIcon), tr("Zoom to selected"), [=]() { graphicsView->zoomToSelected(); });
}

void MainWindow::createActionsSDS()
{
    QToolBar* toolBar = addToolBar(tr("Selection"));
    toolBar->setObjectName(QStringLiteral("s"));
    // s->setMovable(false);
    QAction* action = toolBar->addAction(Icon(SelectAllIcon), tr("Select all"), this, &MainWindow::selectAll);
    action->setShortcut(QKeySequence::SelectAll);

    //    action = toolBar->addAction(Icon(CloseIcon), tr("Redo"), this, &MainWindow::redo);
    //    action->setShortcut(QKeySequence::Redo);

    // action = s->addAction(QIcon::fromTheme("layer-delete"), tr("Delete selected"), [=]() {
    // QList<QGraphicsItem*> list;
    // for (QGraphicsItem* item : MyScene::self->items())
    // if (item->isSelected() && item->type() != DrillItemType)
    // list << item;
    // if (list.size() && QMessageBox::question(this,
    //"", "Do you really want to delete the selected items?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
    // for (QGraphicsItem* item : list)
    // if (item->isSelected() && item->type() != DrillItemType)
    // delete item;
    // MyScene::self->setSceneRect(MyScene::self->itemsBoundingRect());
    // MyScene::self->update();
    // MainWindow::self->zero()->resetPos();
    // MainWindow::self->home()->resetPos();
    // Shtift::shtifts()[0]->resetPos();
    // }
    // });
    // action->setShortcut(QKeySequence::Delete);
}

void MainWindow::createActionsToolPath()
{
    toolpathToolBar = addToolBar(tr("Toolpath"));
    //toolpathToolBar->setIconSize(QSize(24, 24));
    toolpathToolBar->setObjectName(QStringLiteral("toolpathToolBar"));

    // toolpathToolBar->setMovable(false);
    dockWidget = new DockWidget(this);
    connect(dockWidget, &DockWidget::visibilityChanged, [=](bool visible) { if (!visible) resetActions(); });
    dockWidget->setObjectName(QStringLiteral("dwCreatePath"));
    addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    dockWidget->hide();
    // connect(dockWidget, &QDockWidget::visibilityChanged, [&](bool visible) {
    // if (!visible) {
    // for (QAction* action : toolpathActionList)
    // action->setChecked(false);
    // if (dockWidget->widget() != nullptr)
    // dockWidget->widget()->deleteLater();
    // dockWidget->toggleViewAction()->setVisible(false);
    // }
    // });

    toolpathActionList.append(toolpathToolBar->addAction(Icon(PathProfileIcon), tr("Profile"), [=] { createDockWidget(new ProfileForm(dockWidget), Profile); }));
    toolpathActionList.append(toolpathToolBar->addAction(Icon(PathPocketIcon), tr("Pocket"), [=] { createDockWidget(new PocketForm(dockWidget), Pocket); }));
    toolpathActionList.append(toolpathToolBar->addAction(Icon(PathVoronoiIcon), tr("Voronoi"), [=] { createDockWidget(new VoronoiForm(dockWidget), Voronoi); }));
    toolpathActionList.append(toolpathToolBar->addAction(Icon(PathThermalIcon), tr("Thermal Insulation"), [=] { createDockWidget(new ThermalForm(dockWidget), Thermal); }));
    toolpathActionList.append(toolpathToolBar->addAction(Icon(PathDrillIcon), tr("Drilling"), [=] { createDockWidget(new DrillForm(dockWidget), Drill); }));
    toolpathActionList.append(toolpathToolBar->addAction(Icon(GCodePropertiesIcon), tr("G-Code Properties"), [=] { createDockWidget(new GCodePropertiesForm(dockWidget), GCodeProperties); }));

    toolpathToolBar->addSeparator();
    for (QAction* action : toolpathActionList)
        action->setCheckable(true);

#ifdef QT_DEBUG
    QTimer::singleShot(10, [=] { toolpathActionList[Profile]->trigger(); });
#else
    QTimer::singleShot(10, [=] { toolpathActionList[GCodeProperties]->trigger(); });
#endif

    toolpathToolBar->addAction(Icon(ToolDatabaseIcon), tr("Tool Base"), [=] {
        ToolDatabase tdb(this, {});
        tdb.exec();
    });
    toolpathToolBar->addSeparator();
    toolpathToolBar->addAction(Icon(AutoRefpointsIcon), tr("Autoplace All Refpoints"), [=] {
        QList<bool> selected;
        for (QGraphicsItem* item : Scene::self->items()) {
            selected.append(item->isSelected());
            if (item->isVisible())
                item->setSelected(true);
        }

        GCodePropertiesForm::homePoint->resetPos();
        GCodePropertiesForm::zeroPoint->resetPos();
        Shtift::shtifts().first()->resetPos();
        for (QGraphicsItem* item : Scene::self->items())
            item->setSelected(selected.takeFirst());
        graphicsView->zoomFit();
    });
}

void MainWindow::createActionsGraphics()
{
    QToolBar* tb = addToolBar(tr("Graphics Items"));
    tb->setObjectName("GraphicsItemsToolBar");
    tb->setEnabled(false);
    // tb->setMovable(false);
    tb->addAction(QIcon::fromTheme("draw-rectangle"), tr("Rect"));
    tb->addAction(QIcon::fromTheme("draw-line"), tr("line"));
    tb->addAction(QIcon::fromTheme("draw-ellipse"), tr("Elipse"));
    tb->addAction(QIcon::fromTheme("draw-ellipse-arc"), tr("Arc"));
    tb->addAction(QIcon::fromTheme("draw-text"), tr("Text"));
    tb->addSeparator();
    tb->addAction(QIcon::fromTheme("path-union"), tr("union"));
    tb->addAction(QIcon::fromTheme("path-difference"), tr("difference"));
    tb->addAction(QIcon::fromTheme("path-exclusion"), tr("exclusion"));
    tb->addAction(QIcon::fromTheme("path-intersection"), tr("intersection"));
}

void MainWindow::createShtiftsPath()
{
    ToolDatabase tdb(this, { Tool::Drill });
    if (tdb.exec()) {
        Tool tool(tdb.tool());

        QPolygonF dst;

        for (Shtift* item : Shtift::shtifts()) {
            item->setFlag(QGraphicsItem::ItemIsMovable, false);
            QPointF point(item->pos());
            if (dst.contains(point))
                continue;
            dst.append(point);
        }

        qDebug() << dst.size();

        QSettings settings;
        double depth = QInputDialog::getDouble(this, "", tr("Set Depth"), settings.value("Shtift/depth").toDouble(), 0, 100, 2);
        if (depth == 0.0)
            return;
        settings.setValue("Shtift/depth", depth);

        GCode::File* gcode = new GCode::File({ toPath(dst) }, tool, depth, Drill);
        gcode->setFileName("Shtift (Tool Id " + QString::number(tool.id()) + ")");
        FileModel::addFile(gcode);
    }
}

void MainWindow::newFile()
{
    if (maybeSave()) {
        closeProject();
        setCurrentFile(QString());
    }
}

void MainWindow::readSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("geometry", QByteArray()).toByteArray());
    restoreState(settings.value("state", QByteArray()).toByteArray());

    lastPath = settings.value("lastPath").toString();
    curFile = settings.value("project").toString();

    loadFile(curFile);
    //loadFile(settings.value("project").toString());
    // for (const QString& file : settings.value("files").toString().split('|', QString::SkipEmptyParts))
    // openFile(file);

    SettingsDialog().readSettings();
    settings.endGroup();
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("lastPath", lastPath);
    //    settings.setValue("files", Project::fileNames());
    settings.setValue("project", curFile);
    settings.endGroup();
}

void MainWindow::selectAll()
{
    if (focusWidget() && focusWidget()->objectName() == "toolTable") {
        static_cast<QTableView*>(focusWidget())->selectAll();
        return;
    } else {
        for (QGraphicsItem* item : Scene::self->items())
            if (item->isVisible())
                item->setSelected(true);
    }
}

void MainWindow::redo()
{
}

void MainWindow::printDialog()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, &QPrintPreviewDialog::paintRequested, [](QPrinter* printer) {
        Scene::self->m_drawPdf = true;
        QRectF rect;
        for (QGraphicsItem* item : Scene::self->items())
            if (item->isVisible() && !item->boundingRect().isNull())
                rect |= item->boundingRect();
        QSizeF size(rect.size());
        printer->setMargins({ 10, 10, 10, 10 });
        printer->setPageSizeMM(size + QSizeF(printer->margins().left + printer->margins().right, printer->margins().top + printer->margins().bottom));
        printer->setResolution(4800);

        QPainter painter(printer);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.setTransform(QTransform().scale(1.0, -1.0));
        painter.translate(0, -(printer->resolution() / 25.4) * size.height());
        Scene::self->render(&painter, QRectF(0, 0, printer->width(), printer->height()), rect, Qt::KeepAspectRatio /*IgnoreAspectRatio*/);
        Scene::self->m_drawPdf = false;
    });
    preview.exec();
}

void MainWindow::onCustomContextMenuRequested(const QPoint& pos)
{
    QMenu menu;
    QAction* a = nullptr;
    QGraphicsItem* item = Scene::self->itemAt(graphicsView->mapToScene(pos), graphicsView->transform());

    if (!item)
        return;

    if (item->type() == ShtiftType) {
        a = menu.addAction(Icon(PathDrillIcon), tr("&Create path for Shtifts"), this, &MainWindow::createShtiftsPath);
        a = menu.addAction(tr("Fixed"), [](bool fl) {
            for (Shtift* item : Shtift::shtifts())
                item->setFlag(QGraphicsItem::ItemIsMovable, !fl);
        });
        a->setCheckable(true);
        a->setChecked(!(Shtift::shtifts()[0]->flags() & QGraphicsItem::ItemIsMovable));
    } else if (dynamic_cast<Point*>(item)) {
        a = menu.addAction(tr("Fixed"), [=](bool fl) { item->setFlag(QGraphicsItem::ItemIsMovable, !fl); });
        a->setCheckable(true);
        a->setChecked(!(item->flags() & QGraphicsItem::ItemIsMovable));
    } else if (item->type() == ThermalType) {
        if (item->flags() & QGraphicsItem::ItemIsSelectable)
            a = menu.addAction(Icon(CloseAllIcon), tr("Exclude from the calculation"), [=] {
                reinterpret_cast<ThermalPreviewItem*>(item)->node()->disable();
            });
        else
            a = menu.addAction(Icon(NewToolIcon), tr("Include in the calculation"), [=] {
                reinterpret_cast<ThermalPreviewItem*>(item)->node()->enable();
            });
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

void MainWindow::resetActions()
{
    for (QAction* action : toolpathActionList)
        action->setChecked(false);
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
    QSettings settings;
    const int count = settings.beginReadArray(recentFilesKey());
    settings.endArray();
    return count > 0;
}

void MainWindow::open()
{
    if (maybeSave()) {
        QStringList files(QFileDialog::getOpenFileNames(this, tr("Open File"), lastPath, tr("Any (*.*);;Gerber/Excellon (*.gbr *.exc);;Project (*.g2g)")));
        for (QString& fileName : files) {
            loadFile(fileName);
        }
    }
}

bool MainWindow::save()
{
    if (curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool MainWindow::saveAs()
{
    QString file(QFileDialog::getSaveFileName(this, tr("Open File"), lastPath, tr("Project (*.g2g)")));
    if (file.isEmpty())
        return false;
    return saveFile(file); //    QFileDialog dialog(this);
    //    dialog.setWindowModality(Qt::WindowModal);
    //    dialog.setAcceptMode(QFileDialog::AcceptSave);
    //    if (dialog.exec() != QDialog::Accepted)
    //        return false;
    //    return saveFile(dialog.selectedFiles().first()); //    QFileDialog dialog(this);
}

void MainWindow::documentWasModified()
{
    setWindowModified(pro->isModified());
}

bool MainWindow::maybeSave()
{
    if (!pro->isModified())
        return true;
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Application"),
            tr("The document has been modified.\n"
               "Do you want to save your changes?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MainWindow::loadFile(const QString& fileName)
{
    if (!QFileInfo(fileName).exists())
        return;
    if (Project::contains(fileName) != -1
        && QMessageBox::warning(this, "", tr("Do you want to reload file?"), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Cancel) {
        return;
    }
    QFile file(fileName);
    if (file.open(QFile::ReadOnly)) {
        lastPath = QFileInfo(fileName).absolutePath();
        Excellon::Parser dp;
        if (dp.isDrillFile(fileName)) {
            Excellon::File* exFile = dp.parseFile(fileName);
            if (exFile) {
                FileModel::addFile(exFile);
                prependToRecentFiles(exFile->name());
                QTimer::singleShot(100, Qt::CoarseTimer, graphicsView, &GraphicsView::zoomFit);
            }
        } else if (fileName.endsWith(".g2g")) {
            Project::open(file);
            setCurrentFile(fileName);
        } else
            emit parseFile(fileName);
        return;
    }
    QMessageBox::warning(this, "", tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(fileName), file.errorString()));

    //    QFile file(fileName);
    //    if (!file.open(QFile::ReadOnly | QFile::Text)) {
    //        QMessageBox::warning(this, tr("Application"),
    //            tr("Cannot read file %1:\n%2.")
    //                .arg(QDir::toNativeSeparators(fileName), file.errorString()));
    //        return;
    //    }

    //    QTextStream in(&file);
    //    QApplication::setOverrideCursor(Qt::WaitCursor);
    //    textEdit->setPlainText(in.readAll());
    //    QApplication::restoreOverrideCursor();

    //    setCurrentFile(fileName);
    //    statusBar()->showMessage(tr("File loaded"), 2000);
}

bool MainWindow::saveFile(const QString& fileName)
{
    QFile file(fileName);
    if (file.open(QFile::WriteOnly)) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        pro->save(file);
        QApplication::restoreOverrideCursor();
        setCurrentFile(fileName);
        statusBar()->showMessage(tr("File saved"), 2000);
        return true;
    }
    QMessageBox::warning(this, tr("Application"), tr("Cannot write file %1:\n%2.").arg(QDir::toNativeSeparators(fileName), file.errorString()));
    return false;
}

void MainWindow::setCurrentFile(const QString& fileName)
{
    static int sequenceNumber = 1;

    isUntitled = fileName.isEmpty();

    if (isUntitled)
        curFile = tr("Untitled%1.g2g").arg(sequenceNumber++);
    else
        curFile = QFileInfo(fileName).canonicalFilePath();

    pro->setModified(false);
    setWindowModified(false);

    if (!isUntitled && windowFilePath() != curFile)
        prependToRecentFiles(curFile);
    m_closeAllAct->setText(tr("&Close project \"%1\"").arg(strippedName(curFile)));
    setWindowFilePath(curFile);
}

void MainWindow::prependToRecentFiles(const QString& fileName)
{
    QSettings settings;
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
    QSettings settings;

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
        loadFile(action->data().toString());
}

QString MainWindow::strippedName(const QString& fullFileName)
{
    return QFileInfo(fullFileName).fileName();
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
    // dockWidget->setWindowTitle(tr("Create Toolpath"));
    dockWidget->show();
}

void MainWindow::contextMenuEvent(QContextMenuEvent* event)
{
    QMainWindow::contextMenuEvent(event);
}

QMenu* MainWindow::createPopupMenu()
{
    QMenu* menu = QMainWindow::createPopupMenu();
    menu->removeAction(dockWidget->toggleViewAction());
    menu->removeAction(toolpathToolBar->toggleViewAction());
    menu->removeAction(treeDockWidget->toggleViewAction());
    menu->addAction(tr("Icon size = 24"), [=]() { setIconSize(QSize(24, 24)); });
    menu->addAction(tr("Icon size = 48"), [=]() { setIconSize(QSize(48, 48)); });
    menu->addAction(tr("Icon size = 72"), [=]() { setIconSize(QSize(72, 72)); });
    return menu;
}

QString MainWindow::fileKey() { return QStringLiteral("file"); }

QString MainWindow::recentFilesKey() { return QStringLiteral("recentFileList"); }
