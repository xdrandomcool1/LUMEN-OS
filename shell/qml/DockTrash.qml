// SPDX-License-Identifier: GPL-3.0-or-later
// Trash tile at the end of the Dock (drop files to move them to the Trash).
import QtQuick
import QtCore
import Qt.labs.folderlistmodel
import Lumen.Design
import Lumen.Motion
import Lumen.Glass
import Lumen.Shell

Item {
    id: trash
    property var dock
    readonly property int index: dock.appCount
    readonly property real size: dock.sizeFor(index)
    width: size + dock.spacing
    height: 1

    FolderListModel {
        id: trashFiles
        folder: StandardPaths.writableLocation(StandardPaths.GenericDataLocation) + "/Trash/files"
        showHidden: true
        showDirs: true
    }
    readonly property bool full: trashFiles.count > 0

    AppIcon {
        id: icon
        size: trash.size
        anchors.horizontalCenter: parent.horizontalCenter
        y: -size
        source: trash.full ? "user-trash-full" : "user-trash"
        scale: drop.containsDrag ? 1.08 : 1
        Behavior on scale { NumberAnimation { duration: Motion.fast } }
    }
    MouseArea {
        x: (parent.width - trash.size) / 2; y: -trash.size; width: trash.size; height: trash.size
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: true
        onClicked: event => {
            if (event.button === Qt.RightButton) {
                const p = icon.mapToItem(null, icon.width / 2, 0)
                ShellState.openMenu("dock-trash", p.x, [
                    { text: qsTr("Open"), action: () => AppsModel.launch("lumen-finder", ["trash:/"]) },
                    { separator: true },
                    { text: qsTr("Empty Trash"), enabled: trash.full, action: () => AppsModel.emptyTrash() }
                ], { above: true, centered: true, y: (trash.dock.targetScreen ? trash.dock.targetScreen.height : 0) - trash.dock.height + p.y })
            } else {
                AppsModel.launch("lumen-finder", ["trash:/"])
            }
        }
    }
    DropArea {
        id: drop
        x: (parent.width - trash.size) / 2; y: -trash.size; width: trash.size; height: trash.size
        onDropped: d => { if (d.hasUrls) AppsModel.trashUrls(d.urls) }
    }
}
