QT += core gui opengl widgets

TARGET = Getber2Gcode
TEMPLATE = app

CONFIG += c++14 #console

FORMS += \
    mainwindow.ui \
    settingsdialog.ui

RESOURCES += \
    res/resources.qrc \

DISTFILES += \
    qdarkstyle/style.qss \


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
    clipper/clipper.hpp \
    clipper/myclipper.h \
    filetree/abstractitem.h \
    filetree/fileitem.h \
    filetree/folderitem.h \
    filetree/milling.h \
    filetree/Model.h \
    filetree/treeview.h \
    forms/drillingtoolpathform.h \
    forms/pockettoolpathform.h \
    forms/toolpathforminterface.h \
    forms/toolpathnameform.h \
    gerber/aperture.h \
    gerber/gerber.h \
    gerber/graphicsitem.h \
    gerber/mathparser.h \
    gerber/parser.h \
    graphicsview/colorsettings.h \
    graphicsview/edid.h \
    graphicsview/mygraphicsscene.h \
    graphicsview/mygraphicsview.h \
    graphicsview/qdruler.h \
    tooldatabase/tool.h \
    tooldatabase/tooldatabase.h \
    tooldatabase/tooledit.h \
    tooldatabase/toolitem.h \
    tooldatabase/toolmodel.h \
    tooldatabase/tooltreeview.h \
    application.h \
    drillforapertureform.h \
    mainwindow.h \
    openingdialog.h \
    settingsdialog.h \
    toolpath/toolpathcreator.h \
    gerber/file.h \
    gcode/gcode.h \
    toolpath/toolpathwidget.h \
    toolpath/profilewidget.h \
    toolpath/widget.h \
    materialsetup.h \
    graphicsview/point.h \
    toolpath/pocketwidget.h

SOURCES += \
    clipper/clipper.cpp \
    clipper/myclipper.cpp \
    filetree/abstractitem.cpp \
    filetree/fileitem.cpp \
    filetree/folderitem.cpp \
    filetree/milling.cpp \
    filetree/model.cpp \
    filetree/treeview.cpp \
    forms/drillingtoolpathform.cpp \
    forms/pockettoolpathform.cpp \
    forms/toolpathforminterface.cpp \
    forms/toolpathnameform.cpp \
    gerber/aperture.cpp \
    gerber/graphicsitem.cpp \
    gerber/mathparser.cpp \
    gerber/parser.cpp \
    graphicsview/colorsettings.cpp \
    graphicsview/edid.cpp \
    graphicsview/mygraphicsscene.cpp \
    graphicsview/mygraphicsview.cpp \
    graphicsview/qdruler.cpp \
    tooldatabase/tool.cpp \
    tooldatabase/tooldatabase.cpp \
    tooldatabase/tooledit.cpp \
    tooldatabase/toolitem.cpp \
    tooldatabase/toolmodel.cpp \
    tooldatabase/tooltreeview.cpp \
    drillforapertureform.cpp \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    toolpath/toolpathcreator.cpp \
    gerber/file.cpp \
    gcode/gcode.cpp \
    toolpath/toolpathwidget.cpp \
    toolpath/profilewidget.cpp \
    toolpath/widget.cpp \
    materialsetup.cpp \
    graphicsview/point.cpp \
    toolpath/pocketwidget.cpp

