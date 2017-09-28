QT       += core gui opengl widgets

TARGET = Gerber2Pdf

TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += G2P

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    ../G2G/clipper/clipper.cpp \
    ../G2G/clipper/myclipper.cpp \
    ../G2G/gerber/gerberaperture.cpp \
    ../G2G/gerber/gerberparser.cpp \
    ../G2G/gerber/mathparser.cpp \
    ../G2G/graphicsview/colorsettings.cpp \
    ../G2G/graphicsview/mygraphicsscene.cpp \
    ../G2G/graphicsview/mygraphicsview.cpp \
    ../G2G/graphicsview/qdruler.cpp
    ../G2G/graphicsview/edid.cpp \

HEADERS += \
        mainwindow.h \
    ../G2G/clipper/clipper.hpp \
    ../G2G/clipper/myclipper.h \
    ../G2G/gerber/gerber.h \
    ../G2G/gerber/gerberaperture.h \
    ../G2G/gerber/gerberparser.h \
    ../G2G/gerber/mathparser.h \
    ../G2G/graphicsview/colorsettings.h \
    ../G2G/graphicsview/mygraphicsscene.h \
    ../G2G/graphicsview/mygraphicsview.h \
    ../G2G/graphicsview/qdruler.h \
    ../G2G/graphicsview/edid.h \


FORMS += \
        mainwindow.ui

win32* {
    LIBS += -lsetupapi -lAdvapi32 -lpsapi
#    RC_FILE = myapp.rc
}
