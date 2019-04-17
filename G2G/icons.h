#ifndef ICONS_H
#define ICONS_H

#include <QIcon>

enum Id {
    PathDrillIcon,
    PathPocketIcon,
    PathProfileIcon,

    MaterialIcon,
    AutoRefpointsIcon,

    FolderIcon,
    ToolFolderIcon,

    HideOtherIcon,

    SettingsIcon,

    ToolDatabaseIcon,
    ToolDrillIcon,
    ToolEndmillIcon,
    ToolEngraverIcon,

    Zoom100Icon,
    ZoomFitIcon,
    ZoomInIcon,
    ZoomOutIcon,
    ZoomToSelectedIcon,
    SelectAll,

    PuttonAddBridgeIcon,
    PuttonCloseIcon,
    PuttonCreateIcon,
    PuttonEditIcon,
    PuttonSelectIcon,

    OpenFileIcon,
    RemoveIcon,
    DeleteIcon,
    CloseAllIcon,
    CloseIcon,
    PrintIcon,
    SaveAllIcon,
    SaveIcon,
    SavePdfIcon,
    ExitIcon,

};

static QIcon Icon(Id id)
{
    switch (id) {
    case PathDrillIcon:
        return QIcon::fromTheme("roll");
    case PathPocketIcon:
        return QIcon::fromTheme("stroke-to-path");
    case PathProfileIcon:
        return QIcon::fromTheme("object-to-path");
    case MaterialIcon:
        return QIcon::fromTheme("node");
    case AutoRefpointsIcon:
        return QIcon::fromTheme("snap-nodes-cusp");

    case FolderIcon:
        return QIcon::fromTheme("folder");
    case ToolFolderIcon:
        return QIcon::fromTheme("folder-sync");

    case HideOtherIcon:
        return QIcon::fromTheme("hint");

    case SettingsIcon:
        return QIcon::fromTheme("configure-shortcuts");

    case ToolDatabaseIcon:
        return QIcon::fromTheme("view-form");
    case ToolDrillIcon:
        return QIcon::fromTheme("stroke-cap-butt");
    case ToolEndmillIcon:
        return QIcon::fromTheme("stroke-cap-round");
    case ToolEngraverIcon:
        return QIcon::fromTheme("stroke-cap-square");

    case Zoom100Icon:
        return QIcon::fromTheme("zoom-original");
    case ZoomFitIcon:
        return QIcon::fromTheme("zoom-fit-best");
    case ZoomInIcon:
        return QIcon::fromTheme("zoom-in");
    case ZoomOutIcon:
        return QIcon::fromTheme("zoom-out");
    case ZoomToSelectedIcon:
    case SelectAll:
        return QIcon::fromTheme("edit-select-all");

    case PuttonAddBridgeIcon:
        return QIcon::fromTheme("edit-cut");
    case PuttonCloseIcon:
        return QIcon::fromTheme("window-close");
    case PuttonCreateIcon:
        return QIcon::fromTheme("document-export");
    case PuttonEditIcon:
        return QIcon::fromTheme("document-edit");
    case PuttonSelectIcon:
        return QIcon::fromTheme("view-form");
        //return QIcon::fromTheme("tools-wizard");

    case OpenFileIcon:
        return QIcon::fromTheme("document-open");
    case RemoveIcon:
        return QIcon::fromTheme("list-remove");
    case DeleteIcon:
        return QIcon::fromTheme("edit-delete");
    case CloseAllIcon:
        return QIcon::fromTheme("list-remove");
    case CloseIcon:
        return QIcon::fromTheme("document-close");
    case PrintIcon:
        return QIcon::fromTheme("list-remove");
    case SaveAllIcon:
        return QIcon::fromTheme("document-save-all");
    case SaveIcon:
        return QIcon::fromTheme("document-save");
    case SavePdfIcon:
        return QIcon::fromTheme("acrobat");
    case ExitIcon:
        return QIcon::fromTheme("application-exit");
    }
    return QIcon();
}

#endif // ICONS_H
