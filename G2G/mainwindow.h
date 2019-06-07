#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "point.h"
#include "ui_mainwindow.h"
#include <QSettings>
#include <QThread>
#include <qevent.h>

namespace Gerber {
class Parser;
}

class DockWidget;
class Project;

class MainWindow : public QMainWindow, private Ui::MainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();

    void loadFile(const QString& fileName);

    QAction* closeAllAct = nullptr;
    QAction* exportPdfAct = nullptr;
    static MainWindow* self;

    void resetActions();

signals:
    void parseFile(const QString& filename);

private:
    enum { MaxRecentFiles = 20 };
    bool hasRecentFiles();
    inline QString fileKey() { return QStringLiteral("file"); }
    inline QString recentFilesKey() { return QStringLiteral("recentFileList"); }
    void updateRecentFileActions();
    void prependToRecentFiles(const QString& fileName);
    void openRecentFile();
    void setRecentFilesVisible(bool visible);
    void writeRecentFiles(const QStringList& files, QSettings& settings);

    QString lastPath;
    Gerber::Parser* gerberParser;
    QThread gerberThread;

    //////////////////////

    QString strippedName(const QString& fullFileName);
    QStringList readRecentFiles(QSettings& settings);

    void init();
    void about();
    void closeProject();

    // create actions
    void createActions();
    void createActionsFile();
    void createActionsService();
    void createActionsHelp();
    void createActionsZoom();
    void createActionsSDS();
    void createActionsToolPath();
    void createActionsGraphics();

    void createDockWidget(QWidget* dwContent, int type);
    void createStatusBar();
    void createShtifts();
    void newFile();
    void open();
    void saveSelectedToolpaths();
    void setCurrentFile(const QString& fileName);
    void showSettingsDialog();
    void readSettings();
    void writeSettings();
    void selectAll();
    void redo();
    void print();
    void on_customContextMenuRequested(const QPoint& pos);

    QAction* recentFileActs[MaxRecentFiles + 1];
    QAction* recentFileSeparator;
    QAction* recentFileSubMenuAct;

    QString curFile;
    bool isUntitled;

    void fileProgress(const QString& fileName, int max, int value);
    void fileError(const QString& fileName, const QString& error);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    DockWidget* dockWidget;
    QMenu* fileMenu;
    QMenu* helpMenu;
    QMenu* recentMenu;
    QMenu* serviceMenu;
    QToolBar* fileToolBar;
    QToolBar* toolpathToolBar;
    QToolBar* zoomToolBar;
    QVector<QAction*> toolpathActionList;

    Project* pro;

    // QWidget interface
protected:
    virtual void contextMenuEvent(QContextMenuEvent* event) override;

    // QMainWindow interface
public:
    virtual QMenu* createPopupMenu() override;

private:
    /*
    MainWindow()
    {
        createActions();
        createStatusBar();
        readSettings();
        connect(textEdit->document(), &QTextDocument::contentsChanged, this, &MainWindow::documentWasModified);
#ifndef QT_NO_SESSIONMANAGER
        QGuiApplication::setFallbackSessionManagementEnabled(false);
        connect(qApp, &QGuiApplication::commitDataRequest, this, &MainWindow::commitData);
#endif
        setCurrentFile(QString());
        setUnifiedTitleAndToolBarOnMac(true);
    }

    void closeEvent(QCloseEvent* event)
    {
        if (maybeSave()) {
            writeSettings();
            event->accept();
        } else {
            event->ignore();
        }
    }

    void newFile()

    {
        if (maybeSave()) {
            textEdit->clear();
            setCurrentFile(QString());
        }
    }

    void open()

    {
        if (maybeSave()) {
            QString fileName = QFileDialog::getOpenFileName(this);
            if (!fileName.isEmpty())
                loadFile(fileName);
        }
    }
*/
    bool save();
    bool saveAs();
    /*
    void about()

    {
        QMessageBox::about(this, tr("About Application"),
            tr("The <b>Application</b> example demonstrates how to "
               "write modern GUI applications using Qt, with a menu bar, "
               "toolbars, and a status bar."));
    }

    void documentWasModified()

    {
        setWindowModified(textEdit->document()->isModified());
    }

    void createActions()

    {

        QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
        QToolBar* fileToolBar = addToolBar(tr("File"));
        const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
        QAction* newAct = new QAction(newIcon, tr("&New"), this);
        newAct->setShortcuts(QKeySequence::New);
        newAct->setStatusTip(tr("Create a new file"));
        connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
        fileMenu->addAction(newAct);
        fileToolBar->addAction(newAct);

        const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
        QAction* openAct = new QAction(openIcon, tr("&Open..."), this);
        openAct->setShortcuts(QKeySequence::Open);
        openAct->setStatusTip(tr("Open an existing file"));
        connect(openAct, &QAction::triggered, this, &MainWindow::open);
        fileMenu->addAction(openAct);
        fileToolBar->addAction(openAct);

        const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
        QAction* saveAct = new QAction(saveIcon, tr("&Save"), this);
        saveAct->setShortcuts(QKeySequence::Save);
        saveAct->setStatusTip(tr("Save the document to disk"));
        connect(saveAct, &QAction::triggered, this, &MainWindow::save);
        fileMenu->addAction(saveAct);
        fileToolBar->addAction(saveAct);

        const QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
        QAction* saveAsAct = fileMenu->addAction(saveAsIcon, tr("Save &As..."), this, &MainWindow::saveAs);
        saveAsAct->setShortcuts(QKeySequence::SaveAs);
        saveAsAct->setStatusTip(tr("Save the document under a new name"));

        fileMenu->addSeparator();

        const QIcon exitIcon = QIcon::fromTheme("application-exit");
        QAction* exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), this, &QWidget::close);
        exitAct->setShortcuts(QKeySequence::Quit);

        exitAct->setStatusTip(tr("Exit the application"));

        QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
        QToolBar* editToolBar = addToolBar(tr("Edit"));
//!
#ifndef QT_NO_CLIPBOARD
        const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(":/images/cut.png"));
        QAction* cutAct = new QAction(cutIcon, tr("Cu&t"), this);

        cutAct->setShortcuts(QKeySequence::Cut);
        cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                                "clipboard"));
        connect(cutAct, &QAction::triggered, textEdit, &QPlainTextEdit::cut);
        editMenu->addAction(cutAct);
        editToolBar->addAction(cutAct);

        const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/images/copy.png"));
        QAction* copyAct = new QAction(copyIcon, tr("&Copy"), this);
        copyAct->setShortcuts(QKeySequence::Copy);
        copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                                 "clipboard"));
        connect(copyAct, &QAction::triggered, textEdit, &QPlainTextEdit::copy);
        editMenu->addAction(copyAct);
        editToolBar->addAction(copyAct);

        const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(":/images/paste.png"));
        QAction* pasteAct = new QAction(pasteIcon, tr("&Paste"), this);
        pasteAct->setShortcuts(QKeySequence::Paste);
        pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                                  "selection"));
        connect(pasteAct, &QAction::triggered, textEdit, &QPlainTextEdit::paste);
        editMenu->addAction(pasteAct);
        editToolBar->addAction(pasteAct);

        menuBar()->addSeparator();

#endif // !QT_NO_CLIPBOARD

        QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
        QAction* aboutAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
        aboutAct->setStatusTip(tr("Show the application's About box"));

        QAction* aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
        aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));

#ifndef QT_NO_CLIPBOARD
        cutAct->setEnabled(false);

        copyAct->setEnabled(false);
        connect(textEdit, &QPlainTextEdit::copyAvailable, cutAct, &QAction::setEnabled);
        connect(textEdit, &QPlainTextEdit::copyAvailable, copyAct, &QAction::setEnabled);
#endif // !QT_NO_CLIPBOARD
    }

    void createStatusBar()

    {
        statusBar()->showMessage(tr("Ready"));
    }

    void readSettings()

    {
        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
        const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
        if (geometry.isEmpty()) {
            const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
            resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
            move((availableGeometry.width() - width()) / 2,
                (availableGeometry.height() - height()) / 2);
        } else {
            restoreGeometry(geometry);
        }
    }

    void writeSettings()

    {
        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
        settings.setValue("geometry", saveGeometry());
    }
*/
    bool maybeSave();

    //    void loadFile(const QString& fileName);

    bool saveFile(const QString& fileName);

    /*
    QString strippedName(const QString& fullFileName)

    {
        return QFileInfo(fullFileName).fileName();
    }

#ifndef QT_NO_SESSIONMANAGER
    void commitData(QSessionManager& manager)
    {
        if (manager.allowsInteraction()) {
            if (!maybeSave())
                manager.cancel();
        } else {
            // Non-interactive: save without asking
            if (textEdit->document()->isModified())
                save();
        }
    }
#endif
*/
};

class DockWidget : public QDockWidget {
    Q_OBJECT
public:
    DockWidget(QWidget* parent = nullptr)
        : QDockWidget(parent)
    {
    }
    virtual ~DockWidget() {}

    // QWidget interface
protected:
    void closeEvent(QCloseEvent* event) override
    {
        if (widget())
            delete widget();
        event->accept();
    }
};

#endif // MAINWINDOW_H
