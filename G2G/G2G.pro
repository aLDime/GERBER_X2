QT += core gui opengl widgets

TARGET = Getber2Gcode
TEMPLATE = app

CONFIG += c++17 #console

QMAKE_CXXFLAGS += /std:c++17

FORMS += \
    mainwindow.ui \
    settingsdialog.ui \
    tooldatabase/tooledit.ui \
    tooldatabase/tooleditform.ui \
    forms/materialsetupform.ui \
    forms/drillform.ui \
    forms/profileform.ui \
    forms/pocketform.ui

RESOURCES += \
    res/resources.qrc \

DISTFILES += \
    qdarkstyle/style.qss \

include(../graphicsview/mygraphicsview.pri)
include(../gerber/gerber.pri)
include(../clipper/clipper.pri)


DEFINES += QT_DEPRECATED_WARNINGS G2G
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

ICON = res/toolpath/raster_climb.png
#macx: ICON = resources/icon.icns
win32-msvc* {
    LIBS += -lsetupapi -lAdvapi32
    RC_FILE = myapp.rc
}
win32* {
    LIBS += -lsetupapi -lAdvapi32 -lpsapi
    RC_FILE = myapp.rc
}

HEADERS += \
    application.h \
    drillforapertureform.h \
    filetree/abstractitem.h \
    filetree/filemodel.h \
    filetree/folderitem.h \
    filetree/gcodeitem.h \
    filetree/gerberitem.h \
    filetree/treeview.h \
    forms/drillform.h \
    forms/materialsetupform.h \
    forms/profileform.h \
    gcode/gcode.h \
    gcode/toolpathcreator.h \
    mainwindow.h \
    openingdialog.h \
    point.h \
    settingsdialog.h \
    tooldatabase/tool.h \
    tooldatabase/tooldatabase.h \
    tooldatabase/tooleditform.h \
    tooldatabase/toolitem.h \
    tooldatabase/toolmodel.h \
    tooldatabase/tooltreeview.h \
    forms/pocketform.h \
    forms/toolpathutil.h


SOURCES += \
    drillforapertureform.cpp \
    filetree/abstractitem.cpp \
    filetree/filemodel.cpp \
    filetree/folderitem.cpp \
    filetree/gcodeitem.cpp \
    filetree/gerberitem.cpp \
    filetree/treeview.cpp \
    forms/drillform.cpp \
    forms/materialsetupform.cpp \
    forms/profileform.cpp \
    gcode/gcode.cpp \
    gcode/toolpathcreator.cpp \
    main.cpp \
    mainwindow.cpp \
    point.cpp \
    settingsdialog.cpp \
    tooldatabase/tool.cpp \
    tooldatabase/tooldatabase.cpp \
    tooldatabase/tooleditform.cpp \
    tooldatabase/toolitem.cpp \
    tooldatabase/toolmodel.cpp \
    tooldatabase/tooltreeview.cpp \
    forms/pocketform.cpp \
    forms/toolpathutil.cpp



