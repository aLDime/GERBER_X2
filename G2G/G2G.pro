QT += core gui opengl widgets printsupport

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
    voroni/jc_voronoi.cpp



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

DISTFILES += \
    ../icons/breeze/actions/16_/acrobat.svg \
    ../icons/breeze/actions/16_/application-exit.svg \
    ../icons/breeze/actions/16_/configure-shortcuts.svg \
    ../icons/breeze/actions/16_/document-close.svg \
    ../icons/breeze/actions/16_/document-edit-decrypt-verify.svg \
    ../icons/breeze/actions/16_/document-edit-decrypt.svg \
    ../icons/breeze/actions/16_/document-edit-encrypt.svg \
    ../icons/breeze/actions/16_/document-edit-sign-encrypt.svg \
    ../icons/breeze/actions/16_/document-edit-sign.svg \
    ../icons/breeze/actions/16_/document-edit.svg \
    ../icons/breeze/actions/16_/document-export.svg \
    ../icons/breeze/actions/16_/document-open-recent.svg \
    ../icons/breeze/actions/16_/document-open-remote.svg \
    ../icons/breeze/actions/16_/document-open.svg \
    ../icons/breeze/actions/16_/document-save-all.svg \
    ../icons/breeze/actions/16_/document-save-as.svg \
    ../icons/breeze/actions/16_/document-save.svg \
    ../icons/breeze/actions/16_/draw-ellipse-arc.svg \
    ../icons/breeze/actions/16_/draw-ellipse-segment.svg \
    ../icons/breeze/actions/16_/draw-ellipse-whole.svg \
    ../icons/breeze/actions/16_/draw-ellipse.svg \
    ../icons/breeze/actions/16_/draw-line.svg \
    ../icons/breeze/actions/16_/draw-rectangle.svg \
    ../icons/breeze/actions/16_/draw-text.svg \
    ../icons/breeze/actions/16_/edit-cut.svg \
    ../icons/breeze/actions/16_/edit-delete.svg \
    ../icons/breeze/actions/16_/edit-node.svg \
    ../icons/breeze/actions/16_/edit-select-all-layers.svg \
    ../icons/breeze/actions/16_/edit-select-all.svg \
    ../icons/breeze/actions/16_/folder-new.svg \
    ../icons/breeze/actions/16_/folder-sync.svg \
    ../icons/breeze/actions/16_/format-add-node.svg \
    ../icons/breeze/actions/16_/format-break-node.svg \
    ../icons/breeze/actions/16_/format-connect-node.svg \
    ../icons/breeze/actions/16_/format-disconnect-node.svg \
    ../icons/breeze/actions/16_/format-join-node.svg \
    ../icons/breeze/actions/16_/games-hint.svg \
    ../icons/breeze/actions/16_/go-parent-folder.svg \
    ../icons/breeze/actions/16_/hint.svg \
    ../icons/breeze/actions/16_/list-remove-user.svg \
    ../icons/breeze/actions/16_/list-remove.svg \
    ../icons/breeze/actions/16_/node-segment-curve.svg \
    ../icons/breeze/actions/16_/node-segment-line.svg \
    ../icons/breeze/actions/16_/node-transform.svg \
    ../icons/breeze/actions/16_/node-type-auto-smooth.svg \
    ../icons/breeze/actions/16_/node-type-cusp.svg \
    ../icons/breeze/actions/16_/node-type-smooth.svg \
    ../icons/breeze/actions/16_/node-type-symmetric.svg \
    ../icons/breeze/actions/16_/node.svg \
    ../icons/breeze/actions/16_/object-to-path.svg \
    ../icons/breeze/actions/16_/path-difference.svg \
    ../icons/breeze/actions/16_/path-exclusion.svg \
    ../icons/breeze/actions/16_/path-intersection.svg \
    ../icons/breeze/actions/16_/path-reverse.svg \
    ../icons/breeze/actions/16_/path-union.svg \
    ../icons/breeze/actions/16_/roll.svg \
    ../icons/breeze/actions/16_/show-node-handles.svg \
    ../icons/breeze/actions/16_/snap-node.svg \
    ../icons/breeze/actions/16_/snap-nodes-center.svg \
    ../icons/breeze/actions/16_/snap-nodes-cusp.svg \
    ../icons/breeze/actions/16_/snap-nodes-intersection.svg \
    ../icons/breeze/actions/16_/snap-nodes-midpoint.svg \
    ../icons/breeze/actions/16_/snap-nodes-path.svg \
    ../icons/breeze/actions/16_/snap-nodes-rotation-center.svg \
    ../icons/breeze/actions/16_/snap-nodes-smooth.svg \
    ../icons/breeze/actions/16_/stroke-cap-butt.svg \
    ../icons/breeze/actions/16_/stroke-cap-round.svg \
    ../icons/breeze/actions/16_/stroke-cap-square.svg \
    ../icons/breeze/actions/16_/stroke-to-path.svg \
    ../icons/breeze/actions/16_/thermal - копия.svg \
    ../icons/breeze/actions/16_/thermal.svg \
    ../icons/breeze/actions/16_/tools-wizard.svg \
    ../icons/breeze/actions/16_/view-form-action.svg \
    ../icons/breeze/actions/16_/view-form.svg \
    ../icons/breeze/actions/16_/window-close.svg \
    ../icons/breeze/actions/16_/xml-node-delete.svg \
    ../icons/breeze/actions/16_/y-zoom-in.svg \
    ../icons/breeze/actions/22/acrobat.svg \
    ../icons/breeze/actions/22/application-exit.svg \
    ../icons/breeze/actions/22/configure-shortcuts.svg \
    ../icons/breeze/actions/22/document-close.svg \
    ../icons/breeze/actions/22/document-edit-decrypt-verify.svg \
    ../icons/breeze/actions/22/document-edit-decrypt.svg \
    ../icons/breeze/actions/22/document-edit-encrypt.svg \
    ../icons/breeze/actions/22/document-edit-sign-encrypt.svg \
    ../icons/breeze/actions/22/document-edit-sign.svg \
    ../icons/breeze/actions/22/document-edit.svg \
    ../icons/breeze/actions/22/document-export.svg \
    ../icons/breeze/actions/22/document-open-recent.svg \
    ../icons/breeze/actions/22/document-open-remote.svg \
    ../icons/breeze/actions/22/document-open.svg \
    ../icons/breeze/actions/22/document-save-all.svg \
    ../icons/breeze/actions/22/document-save-as-template.svg \
    ../icons/breeze/actions/22/document-save-as.svg \
    ../icons/breeze/actions/22/document-save.svg \
    ../icons/breeze/actions/22/draw-ellipse.svg \
    ../icons/breeze/actions/22/draw-line.svg \
    ../icons/breeze/actions/22/draw-rectangle.svg \
    ../icons/breeze/actions/22/draw-text.svg \
    ../icons/breeze/actions/22/edit-cut.svg \
    ../icons/breeze/actions/22/edit-delete.svg \
    ../icons/breeze/actions/22/edit-node.svg \
    ../icons/breeze/actions/22/edit-select-all.svg \
    ../icons/breeze/actions/22/folder-new.svg \
    ../icons/breeze/actions/22/folder-sync.svg \
    ../icons/breeze/actions/22/format-add-node.svg \
    ../icons/breeze/actions/22/format-break-node.svg \
    ../icons/breeze/actions/22/format-connect-node.svg \
    ../icons/breeze/actions/22/format-disconnect-node.svg \
    ../icons/breeze/actions/22/format-insert-node.svg \
    ../icons/breeze/actions/22/format-join-node.svg \
    ../icons/breeze/actions/22/format-node-corner.svg \
    ../icons/breeze/actions/22/format-node-curve.svg \
    ../icons/breeze/actions/22/format-node-line.svg \
    ../icons/breeze/actions/22/format-node-smooth.svg \
    ../icons/breeze/actions/22/format-node-symmetric.svg \
    ../icons/breeze/actions/22/format-remove-node.svg \
    ../icons/breeze/actions/22/games-hint.svg \
    ../icons/breeze/actions/22/go-parent-folder.svg \
    ../icons/breeze/actions/22/hint.svg \
    ../icons/breeze/actions/22/labplot-zoom-in-x.svg \
    ../icons/breeze/actions/22/labplot-zoom-in-y.svg \
    ../icons/breeze/actions/22/labplot-zoom-out-x.svg \
    ../icons/breeze/actions/22/labplot-zoom-out-y.svg \
    ../icons/breeze/actions/22/list-remove-user.svg \
    ../icons/breeze/actions/22/list-remove.svg \
    ../icons/breeze/actions/22/node-segment-curve.svg \
    ../icons/breeze/actions/22/node-segment-line.svg \
    ../icons/breeze/actions/22/node-transform.svg \
    ../icons/breeze/actions/22/node-type-auto-smooth.svg \
    ../icons/breeze/actions/22/node-type-cusp.svg \
    ../icons/breeze/actions/22/node-type-smooth.svg \
    ../icons/breeze/actions/22/node-type-symmetric.svg \
    ../icons/breeze/actions/22/node.svg \
    ../icons/breeze/actions/22/object-to-path.svg \
    ../icons/breeze/actions/22/path-reverse.svg \
    ../icons/breeze/actions/22/roll.svg \
    ../icons/breeze/actions/22/show-node-handles.svg \
    ../icons/breeze/actions/22/snap-node.svg \
    ../icons/breeze/actions/22/snap-nodes-center.svg \
    ../icons/breeze/actions/22/snap-nodes-intersection.svg \
    ../icons/breeze/actions/22/snap-nodes-rotation-center.svg \
    ../icons/breeze/actions/22/stroke-cap-butt.svg \
    ../icons/breeze/actions/22/stroke-cap-round.svg \
    ../icons/breeze/actions/22/stroke-cap-square.svg \
    ../icons/breeze/actions/22/stroke-to-path.svg \
    ../icons/breeze/actions/22/thermal.svg \
    ../icons/breeze/actions/22/tools-wizard.svg \
    ../icons/breeze/actions/22/view-form-action.svg \
    ../icons/breeze/actions/22/view-form.svg \
    ../icons/breeze/actions/22/window-close.svg \
    ../icons/breeze/actions/22/xml-node-delete.svg \
    ../icons/breeze/actions/22/xml-node-duplicate.svg \
    ../icons/breeze/actions/22/zoom-fit-best.svg \
    ../icons/breeze/actions/22/zoom-in.svg \
    ../icons/breeze/actions/22/zoom-original.svg \
    ../icons/breeze/actions/22/zoom-out.svg \
    ../icons/breeze/actions/24/acrobat.svg \
    ../icons/breeze/actions/24/align-horizontal-node.svg \
    ../icons/breeze/actions/24/align-vertical-node.svg \
    ../icons/breeze/actions/24/application-exit.svg \
    ../icons/breeze/actions/24/configure-shortcuts.svg \
    ../icons/breeze/actions/24/distribute-horizontal-node.svg \
    ../icons/breeze/actions/24/distribute-vertical-node.svg \
    ../icons/breeze/actions/24/document-close.svg \
    ../icons/breeze/actions/24/document-edit-decrypt-verify.svg \
    ../icons/breeze/actions/24/document-edit-decrypt.svg \
    ../icons/breeze/actions/24/document-edit-encrypt.svg \
    ../icons/breeze/actions/24/document-edit-sign-encrypt.svg \
    ../icons/breeze/actions/24/document-edit-sign.svg \
    ../icons/breeze/actions/24/document-edit.svg \
    ../icons/breeze/actions/24/document-export.svg \
    ../icons/breeze/actions/24/document-open-recent.svg \
    ../icons/breeze/actions/24/document-open-remote.svg \
    ../icons/breeze/actions/24/document-open.svg \
    ../icons/breeze/actions/24/document-save-all.svg \
    ../icons/breeze/actions/24/document-save-as-template.svg \
    ../icons/breeze/actions/24/document-save-as.svg \
    ../icons/breeze/actions/24/document-save.svg \
    ../icons/breeze/actions/24/draw-ellipse.svg \
    ../icons/breeze/actions/24/draw-line.svg \
    ../icons/breeze/actions/24/draw-rectangle.svg \
    ../icons/breeze/actions/24/draw-text.svg \
    ../icons/breeze/actions/24/edit-cut.svg \
    ../icons/breeze/actions/24/edit-delete.svg \
    ../icons/breeze/actions/24/edit-node.svg \
    ../icons/breeze/actions/24/edit-select-all.svg \
    ../icons/breeze/actions/24/folder-new.svg \
    ../icons/breeze/actions/24/folder-sync.svg \
    ../icons/breeze/actions/24/format-add-node.svg \
    ../icons/breeze/actions/24/format-break-node.svg \
    ../icons/breeze/actions/24/format-connect-node.svg \
    ../icons/breeze/actions/24/format-disconnect-node.svg \
    ../icons/breeze/actions/24/format-insert-node.svg \
    ../icons/breeze/actions/24/format-join-node.svg \
    ../icons/breeze/actions/24/format-node-corner.svg \
    ../icons/breeze/actions/24/format-node-line.svg \
    ../icons/breeze/actions/24/format-node-smooth.svg \
    ../icons/breeze/actions/24/format-node-symmetric.svg \
    ../icons/breeze/actions/24/format-remove-node.svg \
    ../icons/breeze/actions/24/games-hint.svg \
    ../icons/breeze/actions/24/go-parent-folder.svg \
    ../icons/breeze/actions/24/hint.svg \
    ../icons/breeze/actions/24/list-remove-user.svg \
    ../icons/breeze/actions/24/list-remove.svg \
    ../icons/breeze/actions/24/node - копия.svg \
    ../icons/breeze/actions/24/node-segment-curve.svg \
    ../icons/breeze/actions/24/node-segment-line.svg \
    ../icons/breeze/actions/24/node-transform.svg \
    ../icons/breeze/actions/24/node-type-auto-smooth.svg \
    ../icons/breeze/actions/24/node-type-cusp.svg \
    ../icons/breeze/actions/24/node-type-smooth.svg \
    ../icons/breeze/actions/24/node-type-symmetric.svg \
    ../icons/breeze/actions/24/node.svg \
    ../icons/breeze/actions/24/object-to-path.svg \
    ../icons/breeze/actions/24/path-reverse.svg \
    ../icons/breeze/actions/24/roll.svg \
    ../icons/breeze/actions/24/show-node-handles.svg \
    ../icons/breeze/actions/24/snap-node.svg \
    ../icons/breeze/actions/24/snap-nodes-center.svg \
    ../icons/breeze/actions/24/snap-nodes-cusp.svg \
    ../icons/breeze/actions/24/snap-nodes-intersection.svg \
    ../icons/breeze/actions/24/snap-nodes-midpoint.svg \
    ../icons/breeze/actions/24/snap-nodes-path.svg \
    ../icons/breeze/actions/24/snap-nodes-rotation-center.svg \
    ../icons/breeze/actions/24/snap-nodes-smooth.svg \
    ../icons/breeze/actions/24/stroke-cap-butt.svg \
    ../icons/breeze/actions/24/stroke-cap-round.svg \
    ../icons/breeze/actions/24/stroke-cap-square.svg \
    ../icons/breeze/actions/24/stroke-to-path.svg \
    ../icons/breeze/actions/24/thermal.svg \
    ../icons/breeze/actions/24/tools-wizard.svg \
    ../icons/breeze/actions/24/view-form-action.svg \
    ../icons/breeze/actions/24/view-form.svg \
    ../icons/breeze/actions/24/window-close.svg \
    ../icons/breeze/actions/24/xml-node-delete.svg \
    ../icons/breeze/actions/24/xml-node-duplicate.svg \
    ../icons/breeze/actions/24/zoom-fit-best.svg \
    ../icons/breeze/actions/24/zoom-in.svg \
    ../icons/breeze/actions/24/zoom-original.svg \
    ../icons/breeze/actions/24/zoom-out.svg \
    ../icons/breeze/actions/24/zoom-to-selected - копия.svg \
    ../icons/breeze/actions/24/zoom-to-selected.svg \
    ../icons/breeze/index.theme \
    ../icons/breeze/places/16/folder.svg \
    ../icons/colors.txt \
    ../tools.dat \
    translations/g2g_en.ts \
    translations/g2g_ru.ts

include(../file/file.pri)
include(../clipper/clipper.pri)
include(../excellon/excellon.pri)
include(../gerber/gerber.pri)
include(../graphicsview/mygraphicsview.pri)
include(../voronoi/voronoi.pri)
