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
#include <filetree/gcodeitem.h>
#include <filetree/gerberitem.h>
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
    //    MyGraphicsView::self = graphicsView;

    QVector<Shtift*> shtifts{
        new Shtift(0),
        new Shtift(1),
        new Shtift(2),
        new Shtift(3)
    };

    for (int i = 0; i < 4; ++i) {
        shtifts[i]->setBrush(QColor(255, 255, 0, 100));
        shtifts[i]->setToolTip("Штифт " + QString::number(i + 1));
        shtifts[i]->setShtifts(shtifts);
        shtifts[i]->setParent(MyScene::self);
        shtifts[i]->setZValue(std::numeric_limits<qreal>::max() - i);
        MyScene::self->addItem(shtifts[i]);
    }

    m_zeroPoint = new Point(Point::ZERO);
    m_homePoint = new Point(Point::HOME);

    MyScene::self->addItem(m_zeroPoint);
    MyScene::self->addItem(m_homePoint);

    init();

    gerberParser->moveToThread(&gerberThread);
    connect(&gerberThread, &QThread::finished, gerberParser, &QObject::deleteLater);
    connect(gerberParser, &G::Parser::fileReady, treeView, &TreeView::addFile);
    connect(gerberParser, &G::Parser::fileProgress, this, &MainWindow::fileProgress);
    connect(gerberParser, &G::Parser::fileError, this, &MainWindow::fileError);
    connect(this, &MainWindow::parseFile, gerberParser, &G::Parser::parseFile, Qt::QueuedConnection);
    gerberThread.start(QThread::HighestPriority);

    connect(graphicsView, &MyGraphicsView::FileDroped, this, &MainWindow::openFile);

    //    if (0) {
    //        QPainterPath painterPath;
    //        QFont font;
    //        font.setPointSizeF(10);
    //        painterPath.addText(QPointF(1, -23), font, "Gerber X2");
    //        painterPath.addText(QPointF(1, -12), font, "to");
    //        painterPath.addText(QPointF(1, -01), font, "G Code");
    //        QGraphicsPathItem* pathItem = new QGraphicsPathItem(painterPath);
    //        pathItem->setAcceptHoverEvents(true);
    //        pathItem->setBrush(QColor(255, 180, 120));
    //        pathItem->setPen(Qt::NoPen);
    //        QTransform tr = QTransform::fromScale(2, -2);
    //        pathItem->setTransform(tr);
    //        scene->addItem(pathItem);
    //    }

    readSettings();

    QLatin1String styleSheet("QGroupBox, .QFrame {"
                             //"background-color: rgb(255,255,255);"
                             "border: 1px solid gray;"
                             //"border-radius: 5px;"
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
    //    if (QMessageBox::question(this, "", "Вы действительно хотите выйти из программы?", "Нет", "Да") == 1)
    writeSettings();
    //closeFiles();
    for (G::File*& f : GerberItem::gFiles) {
        delete f;
        f = nullptr;
    }
    for (GCode*& f : GcodeItem::gCode) {
        delete f;
        f = nullptr;
    }
    event->accept();
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

    //==================== Selection ====================
    QToolBar* s = addToolBar(tr("Selection"));
    s->setObjectName(QStringLiteral("s"));
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

    dockWidget = new QDockWidget(this);
    dockWidget->setObjectName(QStringLiteral("dwCreatePath"));
    addDockWidget(Qt::RightDockWidgetArea, dockWidget);
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
            createDockWidget(new ProfileForm(), PROFILE);
        }));
    toolpathActionList.append(toolpathToolBar->addAction(QIcon::fromTheme("stroke-to-path"),
        tr("Pocket"), [=] {
            createDockWidget(new PocketForm(), POCKET);
        }));
    toolpathActionList.append(toolpathToolBar->addAction(QIcon::fromTheme("roll"),
        tr("Drilling"), [=] {
            createDockWidget(new DrillForm(), DRILLING);
        }));
    toolpathActionList.append(toolpathToolBar->addAction(QIcon::fromTheme("node"),
        tr("Setup Material "), [=] {
            createDockWidget(new MaterialSetupForm(), MATERIAL_SETUP_FORM);
        }));

    for (QAction* action : toolpathActionList)
        action->setCheckable(true);

    QTimer::singleShot(10, [=] { createDockWidget(new MaterialSetupForm(), MATERIAL_SETUP_FORM); });

    toolpathToolBar->addAction(QIcon::fromTheme("view-form"),
        tr("Tool Base"), [=]() {
            ToolDatabase tdb(this, {});
            tdb.exec();
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
        openFile(lastPath + "/" + file);
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("lastPath", lastPath);
    settings.setValue("files", treeView->files());
}

void MainWindow::fileProgress(const QString& fileName, int max, int value)
{
    static QProgressDialog progress;
    if (!value) {
        progress.setModal(true);
        progress.setMaximum(max);
        progress.setValue(value);
        progress.setLabelText(fileName);
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

    if (treeView->files().contains(fileName)) {
        QMessageBox::warning(this, "", tr("The document is open."));
        return;
    }

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr(""), tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }
    lastPath = QFileInfo(fileName).absolutePath();
    emit parseFile(fileName);
    //    setCurrentFile(fileName);
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

//bool MainWindow::saveFile(const QString& fileName)
//{
//    QFile file(fileName);
//    if (!file.open(QFile::WriteOnly | QFile::Text)) {
//        QMessageBox::warning(this, tr("SDI"),
//            tr("Cannot write file %1:\n%2.")
//                .arg(QDir::toNativeSeparators(fileName), file.errorString()));
//        return false;
//    }
//    QTextStream out(&file);
//    //    QApplication::setOverrideCursor(Qt::WaitCursor);
//    //    out << textEdit->toPlainText();
//    QApplication::restoreOverrideCursor();
//    setCurrentFile(fileName);
//    statusBar()->showMessage(tr("File saved"), 2000);
//    return true;
//}

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
