#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "point.h"
#include "ui_mainwindow.h"
#include <QSettings>
#include <QThread>

namespace G {
class Parser;
}

class MyScene;

class GerberFileHolder;
class MainWindow : public QMainWindow, private Ui::MainWindow {
    Q_OBJECT

public:
    friend class ProfileToolpathForm;
    friend class GerberFileHolder;
    friend class PocketToolpathForm;

    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();
    void openFile(const QString& fileName);

    QAction* closeAllAct;
    static MainWindow* self;

    Point* zero() const;
    Point* home() const;
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
    G::Parser* gerberParser;
    QThread gerberThread;

    //////////////////////

    QString strippedName(const QString& fullFileName);
    QStringList readRecentFiles(QSettings& settings);

    void init();
    void about();
    void closeFiles();
    void createActions();
    void createDockWidget(QWidget* dwContent, int type);
    void createStatusBar();
    void open();
    void setCurrentFile(const QString& fileName);
    void showSettingsDialog();
    void readSettings();
    void writeSettings();

    QAction* recentFileActs[MaxRecentFiles];
    QAction* recentFileSeparator;
    QAction* recentFileSubMenuAct;

    QString curFile;
    bool isUntitled;

    void fileProgress(const QString& fileName, int max, int value);
    void fileError(const QString& fileName, const QString& error);

    Point* m_zeroPoint = nullptr;
    Point* m_homePoint = nullptr;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    QDockWidget* dockWidget;
    QMenu* fileMenu;
    QMenu* helpMenu;
    QMenu* recentMenu;
    QMenu* serviceMenu;
    QToolBar* fileToolBar;
    QToolBar* toolpathToolBar;
    QToolBar* zoomToolBar;
    QVector<QAction*> toolpathActionList;
};

#endif // MAINWINDOW_H
