#include "drillforapertureform.h"
#include "mainwindow.h"
#include "mainwindow.h"
#include "tooldatabase/tooldatabase.h"

#include "gerber/graphicsitem.h"
#include "graphicsview/mygraphicsscene.h"
#include "settingsdialog.h"
#include "toolpathcreator.h"
#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QtWidgets>
#include "forms/drillingtoolpathform.h"
#include "forms/pockettoolpathform.h"
#include "forms/profiletoolpathform.h"
#include "gerberfileholder.h"
#include "gerber/parser.h"

//#include "qt_windows.h"
//#include "Psapi.h"

MainWindow* MainWindow::pMainWindow;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , gerberParser(new G::Parser)
//, fileHolder(new GerberFileHolder(this))
{
    setupUi(this);
    scene = new MyGraphicsScene(this);
    graphicsView->SetScene(scene);
    Init();
    setCurrentFile(QString());

    gerberParser->moveToThread(&gerberThread);
    connect(&gerberThread, &QThread::finished, gerberParser, &QObject::deleteLater);
    connect(this, &MainWindow::parseFile, gerberParser, &G::Parser::parseFile, Qt::QueuedConnection);
    //    connect(gerberParser, &GerberParser::fileReady, fileHolder, &GerberFileHolder::handleFile);
    connect(gerberParser, &G::Parser::fileReady, treeView, &TreeView::addFile);
    gerberThread.start(QThread::HighestPriority);

    connect(graphicsView, &MyGraphicsView::FileDroped, this, &MainWindow::openFile);

    //    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, QCoreApplication::applicationPid());
    //    static QTimer timer;
    //    connect(&timer, &QTimer::timeout,
    //        [=]() {
    //            PROCESS_MEMORY_COUNTERS pmc;
    //            if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
    //                //                qDebug() << "Ошибок стр.: " << (pmc.PageFaultCount);
    //                //                qDebug() << "Макс. использ. памяти (Kb): " << (pmc.PeakWorkingSetSize / 1024);
    //                //                qDebug() << "Выгружаемый пул (макс.): " << (pmc.QuotaPeakPagedPoolUsage);
    //                //                qDebug() << "Выгружаемый пул : " << (pmc.QuotaPagedPoolUsage);
    //                //                qDebug() << "Невыгруж. пул (макс.): " << (pmc.QuotaPeakNonPagedPoolUsage);
    //                //                qDebug() << "Невыгруж. пул : " << (pmc.QuotaNonPagedPoolUsage);
    //                //                qDebug() << "Вирт. память (Kb): " << (pmc.PagefileUsage / 1024);
    //                //                qDebug() << "Макс. вирт. память (Kb): " << (pmc.PeakPagefileUsage / 1024);
    //                qDebug() << "Память (Kb): " << (pmc.WorkingSetSize / 1024.0 / 1024.0);
    //            }
    //        });
    //    timer.start(1000);

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
        scene->addItem(pathItem);
    }
    dwCreatePath->hide();
    pMainWindow = this;

    readSettings();
}

MainWindow::~MainWindow()
{
    gerberThread.quit();
    gerberThread.wait();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    //    if (QMessageBox::question(this, "", "Вы действительно хотите выйти из программы?", "Нет", "Да") == 1) {
    writeSettings();
    event->accept();
    //    }
    //    else {
    //        event->ignore();
    //    }
}

void MainWindow::open()
{
    QStringList files(QFileDialog::getOpenFileNames(this, tr("Open File"), lastPath, tr("Images (*.gbr *.*)")));
    for (QString& fileName : files) {
        openFile(fileName);
    }
}

void MainWindow::closeFiles()
{
    QModelIndex index = treeView->model()->index(0, 0, QModelIndex());
    int rowCount = static_cast<AbstractItem*>(index.internalPointer())->rowCount();
    treeView->model()->removeRows(0, rowCount, index);
    //fileHolder->closeAllFiles();
    scene->deleteLater();
    scene = new MyGraphicsScene(this);
    graphicsView->SetScene(scene);
}

bool MainWindow::save()
{
    return isUntitled ? saveAs() : saveFile(curFile);
}

bool MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
        curFile);
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About G2G"), tr("G2G"));
}

void MainWindow::documentWasModified()
{
    setWindowModified(true);
}

void MainWindow::Init()
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

    action = fileMenu->addAction(QIcon::fromTheme("acrobat"), tr("&Export PDF..."), this, &MainWindow::exportPdf);
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

    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-fit-best"), tr("Zoom fit best"), [=]() { graphicsView->ZoomFit(); });
    action->setShortcut(QKeySequence::FullScreen);
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-original"), tr("Zoom original"), [=]() { graphicsView->Zoom100(); });
    action->setShortcut(tr("Ctrl+0"));
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-in"), tr("Zoom in"), [=]() { graphicsView->ZoomIn(); });
    action->setShortcut(QKeySequence::ZoomIn);
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-out"), tr("Zoom out"), [=]() { graphicsView->ZoomOut(); });
    action->setShortcut(QKeySequence::ZoomOut);

    //==================== toolpathToolBar ====================

    toolpathToolBar = addToolBar(tr("Toolpath"));
    toolpathToolBar->setIconSize(QSize(24, 24));
    toolpathToolBar->setObjectName(QStringLiteral("toolpathToolBar"));

    dwCreatePath = new QDockWidget(this);
    dwCreatePath->setObjectName(QStringLiteral("dwCreatePath"));
    addDockWidget(Qt::RightDockWidgetArea, dwCreatePath);
    enum {
        PROFILE_TOOLPATH_FORM,
        POCKET_TOOLPATH_FORM,
        DRILLING_TOOLPATH_FORM
    };
    static QVector<QAction*> ToolpathActionList;
    auto createDockWidget = [=](QWidget* dwContents, int type) {
        if (dwCreatePath->widget() != nullptr)
            dwCreatePath->widget()->deleteLater();
        dwContents->setObjectName(QStringLiteral("dwContents"));
        dwCreatePath->setWidget(dwContents);
        //dwCreatePath->setFloating(false);
        dwCreatePath->setWindowTitle(tr("Create Toolpath"));
        for (QAction* action : ToolpathActionList) {
            action->setChecked(false);
        }
        ToolpathActionList[type]->setChecked(true);
        dwCreatePath->show();
    };

    connect(dwCreatePath, &QDockWidget::visibilityChanged, [=](bool visible) {
        if (!visible) {
            QTimer::singleShot(100, [=]() {
                if (dwCreatePath->isHidden()) {
                    for (QAction* action : ToolpathActionList) {
                        action->setChecked(false);
                    }
                    if (dwCreatePath->widget() != nullptr)
                        dwCreatePath->widget()->deleteLater();
                }
            });
        }
    });

    action = toolpathToolBar->addAction(QIcon::fromTheme("object-to-path"), tr("Profile"), [=]() { createDockWidget(new ProfileToolpathForm(), PROFILE_TOOLPATH_FORM); });
    ToolpathActionList.append(action);
    //    action->setShortcut(QKeySequence::FullScreen);
    action = toolpathToolBar->addAction(QIcon::fromTheme("stroke-to-path"), tr("Pocket"), [=]() { createDockWidget(new PocketToolpathForm(), POCKET_TOOLPATH_FORM); });
    ToolpathActionList.append(action);
    //    action->setShortcuts(tr("Ctrl+0"));
    action = toolpathToolBar->addAction(QIcon::fromTheme("roll"), tr("Drilling"), [=]() { createDockWidget(new DrillingToolpathForm(), DRILLING_TOOLPATH_FORM); });
    ToolpathActionList.append(action);
    action = toolpathToolBar->addAction(QIcon::fromTheme("view-form"), tr("Tool Base"), [=]() { ToolDatabase tdb(this); tdb.exec(); });
    //    action->setShortcut(QKeySequence::ZoomIn);
    for (QAction* action : ToolpathActionList) {
        action->setCheckable(true);
    }
    //    if (0) { // for tests
    //        //==================== selectionToolBar ====================
    //        selectionToolBar = addToolBar(tr("Selection Mode"));
    //        selectionToolBar->setIconSize(QSize(24, 24));
    //        selectionToolBar->setObjectName(QStringLiteral("selectionToolBar"));
    //        //    action = selectionToolBar->addAction(QIcon::fromTheme("object-to-path"), tr("Profile"),
    //        //        [=]() { createDockWidget(new ProfileToolpathForm(), PROFILE_TOOLPATH_FORM); });
    //        //    //    action->setShortcut(QKeySequence::FullScreen);
    //        //    action = selectionToolBar->addAction(QIcon::fromTheme("stroke-to-path"), tr("Pocket"),
    //        //        [=]() { createDockWidget(new PocketToolpathForm(), POCKET_TOOLPATH_FORM); });
    //        //    //    action->setShortcuts(tr("Ctrl+0"));
    //        //    action = selectionToolBar->addAction(QIcon::fromTheme("roll"), tr("Drilling"),
    //        //        [=]() { createDockWidget(new DrillingToolpathForm(), DRILLING_TOOLPATH_FORM); });
    //        action = selectionToolBar->addAction(QIcon::fromTheme("view-form"), tr("Tool Base"),
    //            [=]() { DrillForApertureForm dfa("",this); dfa.exec(); });
    //    }
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    if (isHidden()) {
        const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
        if (geometry.isEmpty()) {
            const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
            resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
            move((availableGeometry.width() - width()) / 2,
                (availableGeometry.height() - height()) / 2);
        }
        else {
            restoreGeometry(geometry);
        }
        restoreState(settings.value("state", QByteArray()).toByteArray());
    }
    graphicsView->Setup(settings);
    lastPath = settings.value("lastPath").toString();
    QString files = settings.value("files").toString();
    for (const QString& file : files.split('|', QString::SkipEmptyParts)) {
        openFile(file);
    }
}

void MainWindow::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("lastPath", lastPath);
    settings.setValue("files", treeView->files());
}

bool MainWindow::maybeSave()
{
    //if (!textEdit->document()->isModified())
    return true;
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("SDI"),
            tr("The document has been modified.\n"
               "Do you want to save your changes?"),
            QMessageBox::Save | QMessageBox::Discard
                | QMessageBox::Cancel);
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

void MainWindow::openFile(const QString& fileName)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    if (treeView->files().contains(fileName)) {
        QMessageBox::warning(this, "", tr("The document is open."));
        return;
    }

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr(""), tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }
    lastPath = QDir(fileName).path();

    //QApplication::setOverrideCursor(Qt::WaitCursor);
    emit parseFile(fileName);
    //QApplication::restoreOverrideCursor();
    setCurrentFile(fileName);
    //    statusBar()->showMessage(tr("File loaded"), 2000);
}

MyGraphicsScene* MainWindow::getScene() const
{
    return scene;
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
}

void MainWindow::openRecentFile()
{
    if (const QAction* action = qobject_cast<const QAction*>(sender()))
        openFile(action->data().toString());
}

void MainWindow::exportPdf()
{
    QFileDialog fileDialog(this, tr("Export PDF"));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setMimeTypeFilters(QStringList("application/pdf"));
    fileDialog.setDefaultSuffix("pdf");
    fileDialog.selectFile(curFile.left(curFile.lastIndexOf('.')));
    if (fileDialog.exec() != QDialog::Accepted)
        return;

    QSizeF size = scene->itemsBoundingRect().size();

    QPdfWriter pdfWriter(curFile.left(curFile.lastIndexOf('.')) + ".pdf");
    pdfWriter.setPageSizeMM(size);
    QPdfWriter::Margins margins = { 0, 0, 0, 0 };
    pdfWriter.setMargins(margins);
    pdfWriter.setResolution(10000000);

    QPainter painter(&pdfWriter);
    painter.setTransform(QTransform().scale(1.0, -1.0));
    painter.translate(0, -(pdfWriter.resolution() / 25.4) * size.height());
    qDebug() << size << pdfWriter.resolution();
    scene->RenderPdf(&painter);
}

bool MainWindow::saveFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("SDI"),
            tr("Cannot write file %1:\n%2.")
                .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    QTextStream out(&file);
    //    QApplication::setOverrideCursor(Qt::WaitCursor);
    //    out << textEdit->toPlainText();
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

void MainWindow::setCurrentFile(const QString& fileName)
{
    static int sequenceNumber = 1;

    isUntitled = fileName.isEmpty();
    if (isUntitled) {
        curFile = tr("document%1.txt").arg(sequenceNumber++);
    }
    else {
        curFile = QFileInfo(fileName).canonicalFilePath();
    }

    //    textEdit->document()->setModified(false);
    setWindowModified(false);

    if (!isUntitled && windowFilePath() != curFile)
        MainWindow::prependToRecentFiles(curFile);

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
        readSettings();
    }
}

MainWindow* MainWindow::getMainWindow() { return pMainWindow; }
