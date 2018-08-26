#-------------------------------------------------
#
# Project created by QtCreator 2018-08-26T08:52:53
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = G_TEST_FILE_PARSER
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    ../G2G/graphicsview/colorsettings.cpp \
    ../G2G/graphicsview/edid.cpp \
    ../G2G/graphicsview/mygraphicsscene.cpp \
    ../G2G/graphicsview/mygraphicsview.cpp \
    ../G2G/graphicsview/point.cpp \
    ../G2G/graphicsview/qdruler.cpp

HEADERS += \
        mainwindow.h \
    ../G2G/graphicsview/colorsettings.h \
    ../G2G/graphicsview/edid.h \
    ../G2G/graphicsview/mygraphicsscene.h \
    ../G2G/graphicsview/mygraphicsview.h \
    ../G2G/graphicsview/point.h \
    ../G2G/graphicsview/qdruler.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
