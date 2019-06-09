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

    //    static QAction* closeAllAct();
    //    static QAction* exportPdfAct();
    // QMainWindow interface
    virtual QMenu* createPopupMenu() override;

signals:
    void parseFile(const QString& filename);

private:
    enum { MaxRecentFiles = 20 };
    DockWidget* dockWidget = nullptr;

    Gerber::Parser* gerberParser;

    QAction* m_closeAllAct = nullptr;
    QAction* recentFileActs[MaxRecentFiles + 1];
    QAction* recentFileSeparator = nullptr;
    QAction* recentFileSubMenuAct = nullptr;

    QMenu* fileMenu = nullptr;
    QMenu* helpMenu = nullptr;
    QMenu* recentMenu = nullptr;
    QMenu* serviceMenu = nullptr;

    QString curFile;
    QString lastPath;
    QThread gerberThread;

    QToolBar* fileToolBar = nullptr;
    QToolBar* toolpathToolBar = nullptr;
    QToolBar* zoomToolBar = nullptr;

    Project* pro;

    QVector<QAction*> toolpathActionList;

    bool isUntitled;

    static MainWindow* self;

    inline QString fileKey();
    inline QString recentFilesKey();
    void about();
    void closeProject();
    void createDockWidget(QWidget* dwContent, int type);
    void createShtiftsPath();
    void fileError(const QString& fileName, const QString& error);
    void fileProgress(const QString& fileName, int max, int value);
    void init();
    void onCustomContextMenuRequested(const QPoint& pos);
    void openRecentFile();
    void prependToRecentFiles(const QString& fileName);
    void printDialog();
    void readSettings();
    void redo();
    void resetActions();
    void saveSelectedToolpaths();
    void selectAll();
    void setRecentFilesVisible(bool visible);
    void updateRecentFileActions();
    void writeRecentFiles(const QStringList& files, QSettings& settings);
    void writeSettings();

    // create actions
    void createActions();
    void createActionsFile();
    void createActionsService();
    void createActionsHelp();
    void createActionsZoom();
    void createActionsSDS();
    void createActionsToolPath();
    void createActionsGraphics();

    QString strippedName(const QString& fullFileName);
    QStringList readRecentFiles(QSettings& settings);
    bool hasRecentFiles();
    void newFile();
    void open();
    bool save();
    bool saveAs();
    void documentWasModified();
    bool maybeSave();
    void loadFile(const QString& fileName);
    bool saveFile(const QString& fileName);
    void setCurrentFile(const QString& fileName);

    // QWidget interface
protected:
    void closeEvent(QCloseEvent* event) override;
    virtual void contextMenuEvent(QContextMenuEvent* event) override;
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
