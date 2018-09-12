#include <QApplication>
#include <QCommandLineParser>

#include "application.h"
#include "mainwindow.h"

#ifndef linux
#include <qt_windows.h>

#include <gcode/drl.h>
#endif

int main(int argc, char* argv[])
{

    for (int i = 0;
         i < 1;
         ++i) {
        qDebug() << i;
    }
#ifndef linux
    HANDLE hCorvetEvent = CreateEventA(nullptr, FALSE, FALSE, ("Getber2Gcode"));
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        if (argc == 2) {
            QSettings* settings;
            settings = new QSettings("XrSoft", "G2G");
            settings->setValue("AddFile", QString::fromLocal8Bit(argv[1]).replace(QString("//"), QString("/")));
            CloseHandle(hCorvetEvent);
        }
        return 0;
    }
#endif

    //Q_INIT_RESOURCE(resources);
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName("G2G");
    QCoreApplication::setOrganizationName("XrSoft");
    QCoreApplication::setApplicationVersion("0.2.3");

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file(s) to open.");
    parser.process(app);
    QIcon::setThemeSearchPaths({ "../icons/breeze/" });

    QIcon::setThemeName("Breeze");

    MainWindow* mainWin = new MainWindow;
    for (const QString& file : parser.positionalArguments()) {
        mainWin->openFile(file);
    }
    mainWin->show();

//    Drl d;
//    d.parseFile("C:/Users/User/Downloads/gbr/Gerber Test Files/123/d24.drl");
//    d.parseFile("C:/Users/User/Downloads/gbr/Gerber Test Files/123/d24d.drl");
//    d.parseFile("C:/Users/User/Downloads/gbr/Gerber Test Files/123/m32.drl");
//    d.parseFile("C:/Users/User/Downloads/gbr/Gerber Test Files/123/m32d.drl");
//    d.parseFile("C:/Users/User/Downloads/gbr/Gerber Test Files/123/m33d.drl");
//    QTimer::singleShot(100, Qt::CoarseTimer, mainWin, &MainWindow::close);

    return app.exec();
}
