#include <QApplication>
#include <QCommandLineParser>
#ifndef linux
#include <qt_windows.h>

#include <tooldatabase/tooldatabase.h>
#endif
#include "application.h"
#include "mainwindow.h"

int main(int argc, char* argv[])
{
#ifndef linux
    HANDLE hCorvetEvent = CreateEventA(NULL, FALSE, FALSE, ("Getber2Gcode"));
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        if (argc == 2) {
            QSettings* settings;
            settings = new QSettings("XrSoft", "G2G");
            settings->setValue("AddFile", QString::fromLocal8Bit(argv[1]).replace(QString("\\"), QString("/")));
            CloseHandle(hCorvetEvent);
        }
        return 0;
    }
#endif

    Q_INIT_RESOURCE(resources);
    QApplication app(argc, argv);

    //    if (!app.lock())
    //        return -42;

    QCoreApplication::setApplicationName("G2G");
    QCoreApplication::setOrganizationName("XrSoft");
    QCoreApplication::setApplicationVersion("0.2.3");

    //    EditToolForm editTool;
    //    editTool.show();

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file(s) to open.");
    parser.process(app);

    QIcon::setThemeSearchPaths({ "D:/PRO/QT_PROJECTS/icons/breeze/" });
    QIcon::setThemeName("Breeze");

    MainWindow* mainWin = new MainWindow;
    for (const QString& file : parser.positionalArguments()) {
        mainWin->openFile(file);
    }
    mainWin->show();

    return app.exec();
}
