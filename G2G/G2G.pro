QT += core gui opengl widgets

TARGET = Getber2Gcode
TEMPLATE = app

CONFIG += c++17 #console

QMAKE_CXXFLAGS += /std:c++17

FORMS += \
    mainwindow.ui \
    settingsdialog.ui

RESOURCES += \
    res/resources.qrc \

DISTFILES += \
    qdarkstyle/style.qss \

include(../graphicsview/mygraphicsview.pri)


DEFINES += QT_DEPRECATED_WARNINGS
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
    clipper/clipper.hpp \
    clipper/myclipper.h \
    drillforapertureform.h \
    filetree/Model.h \
    filetree/abstractitem.h \
    filetree/fileitem.h \
    filetree/folderitem.h \
    filetree/milling.h \
    filetree/treeview.h \
#    forms/drillingtoolpathform.h \
    forms/materialsetup.h \
#    forms/pockettoolpathform.h \
#    forms/toolpathforminterface.h \
#    forms/toolpathnameform.h \
    gcode/gcode.h \
    gerber/aperture.h \
    gerber/file.h \
    gerber/gerber.h \
    gerber/graphicsitem.h \
    gerber/mathparser.h \
    gerber/parser.h \
#    graphicsview/colorsettings.h \
#    graphicsview/edid.h \
#    graphicsview/mygraphicsscene.h \
#    graphicsview/mygraphicsview.h \
#    graphicsview/point.h \
#    graphicsview/qdruler.h \
    mainwindow.h \
    openingdialog.h \
    settingsdialog.h \
    tooldatabase/tool.h \
    tooldatabase/tooldatabase.h \
    tooldatabase/tooledit.h \
    tooldatabase/toolitem.h \
    tooldatabase/toolmodel.h \
    tooldatabase/tooltreeview.h \
    toolpath/profilewidget.h \
    toolpath/toolpathcreator.h \
    toolpath/toolpathwidget.h \
    toolpath/widget.h \
    toolpath/pocketwidget.h

SOURCES += \
    clipper/clipper.cpp \
    clipper/myclipper.cpp \
    drillforapertureform.cpp \
    filetree/abstractitem.cpp \
    filetree/fileitem.cpp \
    filetree/folderitem.cpp \
    filetree/milling.cpp \
    filetree/model.cpp \
    filetree/treeview.cpp \
#    forms/drillingtoolpathform.cpp \
    forms/materialsetup.cpp \
#    forms/pockettoolpathform.cpp \
#    forms/toolpathforminterface.cpp \
#    forms/toolpathnameform.cpp \
    gcode/gcode.cpp \
    gerber/aperture.cpp \
    gerber/file.cpp \
    gerber/graphicsitem.cpp \
    gerber/mathparser.cpp \
    gerber/parser.cpp \
#    graphicsview/colorsettings.cpp \
#    graphicsview/edid.cpp \
#    graphicsview/mygraphicsscene.cpp \
#    graphicsview/mygraphicsview.cpp \
#    graphicsview/point.cpp \
#    graphicsview/qdruler.cpp \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    tooldatabase/tool.cpp \
    tooldatabase/tooldatabase.cpp \
    tooldatabase/tooledit.cpp \
    tooldatabase/toolitem.cpp \
    tooldatabase/toolmodel.cpp \
    tooldatabase/tooltreeview.cpp \
    toolpath/profilewidget.cpp \
    toolpath/toolpathcreator.cpp \
    toolpath/toolpathwidget.cpp \
    toolpath/widget.cpp \
    toolpath/pocketwidget.cpp

