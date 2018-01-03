#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gerber/gerber.h"
#include "gerber/parser.h"

#include "ui_mainwindow.h"
#include <QSettings>
#include <QThread>

class MyGraphicsScene;
class G::Parser;
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
    //    void OpenFile(QString file);
    //    void OpenFile();
    //    void ClearScene();

    MyGraphicsScene* getScene() const;
    static MainWindow* getMainWindow();
    QAction* closeAllAct;

signals:
    void parseFile(const QString& filename);

private:
    enum { MaxRecentFiles = 20 };

    //    void writeSettings();
    //    void readSettings();
    QString lastPath;
    G::Parser* gerberParser;
    QThread gerberThread;
    //    GerberFileHolder* fileHolder;

    MyGraphicsScene* scene;
    //////////////////////
    void open();
    void closeFiles();

    bool save();
    bool saveAs();
    void about();
    void documentWasModified();
    void Init();
    void createActions();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool maybeSave();
    void setRecentFilesVisible(bool visible);
    static inline QString recentFilesKey() { return QStringLiteral("recentFileList"); }
    static inline QString fileKey() { return QStringLiteral("file"); }
    static QStringList readRecentFiles(QSettings& settings);
    static void writeRecentFiles(const QStringList& files, QSettings& settings);
    bool hasRecentFiles();
    void prependToRecentFiles(const QString& fileName);
    void updateRecentFileActions();
    void openRecentFile();
    void exportPdf();
    bool saveFile(const QString& fileName);
    void setCurrentFile(const QString& fileName);
    QString strippedName(const QString& fullFileName);
    ////////////////////////////////////////////////////
    void showSettingsDialog();
    //QTextEdit* textEdit;

    QAction* recentFileActs[MaxRecentFiles];
    QAction* recentFileSeparator;
    QAction* recentFileSubMenuAct;

    QString curFile;
    bool isUntitled;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    QDockWidget* dwCreatePath;
    QMenu* fileMenu;
    QMenu* helpMenu;
    QMenu* recentMenu;
    QMenu* serviceMenu;
    QToolBar* fileToolBar;
    QToolBar* toolpathToolBar;
    QToolBar* zoomToolBar;
    static MainWindow* pMainWindow;
    QVector<QAction*> toolpathActionList;
};

#endif // MAINWINDOW_H
