#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>

namespace Ui {
class MainWindow;
}
namespace G {
class File;
}
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

signals:
    void parseFile(const QString&);

private:
    Ui::MainWindow* ui;

    void openFile(const QString& fileName);

    QString lastPath;

    QThread gerberThread;
    G::File* m_file = nullptr;

    void fileReady(G::File* m_file);
};

#endif // MAINWINDOW_H
