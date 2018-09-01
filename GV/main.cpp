#include "mainwindow.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QApplication::setApplicationName("GV");
    QApplication::setOrganizationName("XrSoft");
    //    QApplication::setApplicationVersion("0.2.3");

    MainWindow w;
    w.show();

    return a.exec();
}
