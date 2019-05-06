QT += core gui opengl widgets

TARGET = Getber2Gcode
TEMPLATE = app

#QMAKE_CXXFLAGS += /std:c++17

RESOURCES += \
    res/resources.qrc \

DEFINES += QT_DEPRECATED_WARNINGS G2G
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

VERSION = 0.7.3
VER_MAJ = 0
VER_MIN = 7
VER_PAT = 3

ICON = 256.png

#macx: ICON = resources/icon.icns

win32-msvc* {
    LIBS += -lsetupapi -lAdvapi32
    RC_FILE = myapp.rc
}
win32* {
    LIBS += -lsetupapi -lAdvapi32 -lpsapi
    RC_FILE = myapp.rc
}

TRANSLATIONS += \
    translations/g2g_en.ts \
    translations/g2g_ru.ts

HEADERS += \
    aboutform.h \
    application.h \
    colorselector.h \
    doublespinbox.h \
    filetree/abstractnode.h \
    filetree/drillnode.h \
    filetree/fileholder.h \
    filetree/filemodel.h \
    filetree/foldernode.h \
    filetree/gcodenode.h \
    filetree/gerbernode.h \
    filetree/layerdelegate.h \
    filetree/treeview.h \
    forms/drillform.h \
    forms/drillmodel.h \
    forms/materialsetupform.h \
    forms/pocketform.h \
    forms/profileform.h \
    forms/termalform.h \
    forms/termalmodel.h \
    forms/termalpreviewitem.h \
    forms/toolpathutil.h \
    forms/voronoiform.h \
    gcode/gcode.h \
    gcode/toolpathcreator.h \
    gi/boarditem.h \
    gi/bridgeitem.h \
    gi/drillitem.h \
    gi/gerberitem.h \
    gi/graphicsitem.h \
    gi/itemgroup.h \
    gi/pathitem.h \
    gi/rawitem.h \
    gi/ruler.h \
    mainwindow.h \
    openingdialog.h \
    point.h \
    settingsdialog.h \
    tooldatabase/tool.h \
    tooldatabase/tooldatabase.h \
    tooldatabase/tooleditdialog.h \
    tooldatabase/tooleditform.h \
    tooldatabase/toolitem.h \
    tooldatabase/toolmodel.h \
    tooldatabase/tooltreeview.h \
    icons.h \
    forms/previewitem.h \
    voroni/jc_voronoi.h


SOURCES += \
    aboutform.cpp \
    colorselector.cpp \
    doublespinbox.cpp \
    filetree/abstractnode.cpp \
    filetree/drillnode.cpp \
    filetree/fileholder.cpp \
    filetree/filemodel.cpp \
    filetree/foldernode.cpp \
    filetree/gcodenode.cpp \
    filetree/gerbernode.cpp \
    filetree/layerdelegate.cpp \
    filetree/treeview.cpp \
    forms/drillform.cpp \
    forms/drillmodel.cpp \
    forms/materialsetupform.cpp \
    forms/pocketform.cpp \
    forms/profileform.cpp \
    forms/termalform.cpp \
    forms/termalmodel.cpp \
    forms/termalpreviewitem.cpp \
    forms/toolpathutil.cpp \
    forms/voronoiform.cpp \
    gcode/gcode.cpp \
    gcode/toolpathcreator.cpp \
    gi/boarditem.cpp \
    gi/bridgeitem.cpp \
    gi/drillitem.cpp \
    gi/gerberitem.cpp \
    gi/graphicsitem.cpp \
    gi/itemgroup.cpp \
    gi/pathitem.cpp \
    gi/rawitem.cpp \
    gi/ruler.cpp \
    main.cpp \
    mainwindow.cpp \
    point.cpp \
    settingsdialog.cpp \
    tooldatabase/tool.cpp \
    tooldatabase/tooldatabase.cpp \
    tooldatabase/tooleditdialog.cpp \
    tooldatabase/tooleditform.cpp \
    tooldatabase/toolitem.cpp \
    tooldatabase/toolmodel.cpp \
    tooldatabase/tooltreeview.cpp \
    forms/previewitem.cpp \
    voroni/jc_voronoi.cpp



FORMS += \
    aboutform.ui \
    colorselector.ui \
    forms/drillform.ui \
    forms/materialsetupform.ui \
    forms/pocketform.ui \
    forms/profileform.ui \
    forms/termalform.ui \
    forms/voronoiform.ui \
    mainwindow.ui \
    settingsdialog.ui \
    tooldatabase/tooleditdialog.ui \
    tooldatabase/tooleditform.ui \
    tooldatabase/tooldatabase.ui \

DISTFILES += \
    translations/g2g_en.ts \
    translations/g2g_ru.ts

include(../file/file.pri)
include(../clipper/clipper.pri)
include(../excellon/excellon.pri)
include(../gerber/gerber.pri)
include(../graphicsview/mygraphicsview.pri)
include(../voronoi/voronoi.pri)
