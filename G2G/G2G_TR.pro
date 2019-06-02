QT += core gui opengl widgets printsupport

TARGET = Getber2Gcode
TEMPLATE = app

#QMAKE_CXXFLAGS += /std:c++17

RESOURCES += \
    res/resources.qrc \

DEFINES += QT_DEPRECATED_WARNINGS G2G
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

VERSION = 0.7.5
VER_MAJ = 0
VER_MIN = 7
VER_PAT = 5

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
    filetree/Project.h \
    filetree/filemodel.h \
    filetree/foldernode.h \
    filetree/gcodenode.h \
    filetree/gerbernode.h \
    filetree/layerdelegate.h \
    filetree/treeview.h \
    forms/drillform.h \
    forms/drillmodel.h \
    forms/gcodepropertiesform.h \
    forms/pocketform.h \
    forms/profileform.h \
    forms/thermalform.h \
    forms/thermalmodel.h \
    forms/thermalpreviewitem.h \
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
    voroni/jc_voronoi.h \
#    ../clipper/line.h \
#    ../clipper/myclipper.h \
#    ../excellon/excellon.h \
#    ../excellon/excellondialog.h \
#    ../excellon/exfile.h \
#    ../excellon/exparser.h \
#    ../excellon/exvars.h \
#    ../file/abstractfile.h \
#    ../gerber/gbraperture.h \
#    ../gerber/gbrfile.h \
#    ../gerber/gbrparser.h \
#    ../gerber/gbrvars.h \
#    ../gerber/mathparser.h \
#    ../graphicsview/edid.h \
#    ../graphicsview/graphicsview.h \
#    ../graphicsview/qdruler.h \
#    ../graphicsview/scene.h \
#    ../voronoi/cell.h \
#    ../voronoi/circleevent.h \
#    ../voronoi/diagram.h \
#    ../voronoi/edge.h \
#    ../voronoi/halfedge.h \
#    ../voronoi/rbtree.h \
#    ../voronoi/vertex.h \
#    ../voronoi/voronoi.h


SOURCES += \
    aboutform.cpp \
    colorselector.cpp \
    doublespinbox.cpp \
    filetree/abstractnode.cpp \
    filetree/drillnode.cpp \
    filetree/Project.cpp \
    filetree/filemodel.cpp \
    filetree/foldernode.cpp \
    filetree/gcodenode.cpp \
    filetree/gerbernode.cpp \
    filetree/layerdelegate.cpp \
    filetree/treeview.cpp \
    forms/drillform.cpp \
    forms/drillmodel.cpp \
    forms/gcodepropertiesform.cpp \
    forms/pocketform.cpp \
    forms/profileform.cpp \
    forms/thermalform.cpp \
    forms/thermalmodel.cpp \
    forms/thermalpreviewitem.cpp \
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
    voroni/jc_voronoi.cpp \
#    ../clipper/clipper.cpp \
#    ../clipper/line.cpp \
#    ../clipper/myclipper.cpp \
#    ../excellon/excellondialog.cpp \
#    ../excellon/exfile.cpp \
#    ../excellon/exformatstate.cpp \
#    ../excellon/exparser.cpp \
#    ../file/abstractfile.cpp \
#    ../gerber/gbraperture.cpp \
#    ../gerber/gbrfile.cpp \
#    ../gerber/gbrparser.cpp \
#    ../gerber/mathparser.cpp \
#    ../graphicsview/edid.cpp \
#    ../graphicsview/graphicsview.cpp \
#    ../graphicsview/qdruler.cpp \
#    ../graphicsview/scene.cpp \
#    ../voronoi/cell.cpp \
#    ../voronoi/diagram.cpp \
#    ../voronoi/edge.cpp \
#    ../voronoi/halfedge.cpp \
#    ../voronoi/rbtree.cpp \
#    ../voronoi/vertex.cpp \
#    ../voronoi/voronoi.cpp




FORMS += \
    aboutform.ui \
    colorselector.ui \
    forms/drillform.ui \
    forms/gcodepropertiesform.ui \
    forms/pocketform.ui \
    forms/profileform.ui \
    forms/thermalform.ui \
    forms/voronoiform.ui \
    mainwindow.ui \
    settingsdialog.ui \
    tooldatabase/tooleditdialog.ui \
    tooldatabase/tooleditform.ui \
    tooldatabase/tooldatabase.ui \
    ../excellon/excellondialog.ui

