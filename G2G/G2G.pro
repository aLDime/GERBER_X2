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
    forms/materialsetupform.ui

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
    filetree/fileitem.h \
    filetree/folderitem.h \
    filetree/milling.h \
    filetree/treeview.h \
    forms/materialsetupform.h \
    gcode/gcode.h \
    mainwindow.h \
    openingdialog.h \
    settingsdialog.h \
    tooldatabase/tool.h \
    tooldatabase/tooldatabase.h \
    tooldatabase/tooleditform.h \
    tooldatabase/toolitem.h \
    tooldatabase/toolmodel.h \
    tooldatabase/tooltreeview.h \
    toolpath/pocketwidget.h \
    toolpath/profilewidget.h \
    toolpath/toolpathcreator.h \
    toolpath/toolpathwidget.h \
    toolpath/widget.h \
#    forms/drillingtoolpathform.h \
#    forms/pockettoolpathform.h \
#    forms/toolpathforminterface.h \
#    forms/toolpathnameform.h \
    filetree/filemodel.h

SOURCES += \
    drillforapertureform.cpp \
    filetree/abstractitem.cpp \
    filetree/fileitem.cpp \
    filetree/folderitem.cpp \
    filetree/milling.cpp \
    filetree/treeview.cpp \
    forms/materialsetupform.cpp \
    gcode/gcode.cpp \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    tooldatabase/tool.cpp \
    tooldatabase/tooldatabase.cpp \
    tooldatabase/tooleditform.cpp \
    tooldatabase/toolitem.cpp \
    tooldatabase/toolmodel.cpp \
    tooldatabase/tooltreeview.cpp \
    toolpath/pocketwidget.cpp \
    toolpath/profilewidget.cpp \
    toolpath/toolpathcreator.cpp \
    toolpath/toolpathwidget.cpp \
    toolpath/widget.cpp \
#    forms/drillingtoolpathform.cpp \
#    forms/pockettoolpathform.cpp \
#    forms/toolpathforminterface.cpp \
#    forms/toolpathnameform.cpp \
    filetree/filemodel.cpp

