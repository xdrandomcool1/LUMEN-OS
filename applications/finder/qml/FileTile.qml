// SPDX-License-Identifier: GPL-3.0-or-later
// Icon-view item.
import QtQuick
import Lumen.Design
import Lumen.Motion
import Lumen.Controls

Item {
    id: tile
    required property int index
    required property var model
    property var browser
    property var folder
    property real viewWidth: 108
    readonly property bool selected: browser.selection[index] === true
    width: viewWidth; height: 112

    Rectangle {
        anchors.horizontalCenter: icon.horizontalCenter; anchors.verticalCenter: icon.verticalCenter
        width: 76; height: 76; radius: 10
        color: Theme.controlFillPressed
        visible: tile.selected || drop.containsDrag
    }
    FileIcon {
        id: icon
        anchors.horizontalCenter: parent.horizontalCenter
        y: 6
        size: 64
        iconName: tile.model.iconName
        url: tile.model.url
        isImage: tile.model.isImage
        opacity: tile.model.isHidden ? 0.5 : 1
    }
    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 80
        width: Math.min(tile.width - 6, nameText.contentWidth + 10); height: nameText.contentHeight + 4; radius: 5
        color: tile.selected ? Theme.selection : "transparent"
        visible: tile.browser.renameRow !== tile.index
        LText {
            id: nameText
            anchors.centerIn: parent
            width: tile.width - 14
            text: tile.model.name
            style: LText.Callout
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            maximumLineCount: 2
            elide: Text.ElideMiddle
            color: tile.selected ? "white" : Theme.label
        }
    }
    TextField {
        visible: tile.browser.renameRow === tile.index
        anchors.horizontalCenter: parent.horizontalCenter
        y: 78; width: tile.width - 4
        horizontalAlignment: TextInput.AlignHCenter
        onVisibleChanged: if (visible) { text = tile.model.name; forceActiveFocus(); const dot = text.lastIndexOf("."); select(0, dot > 0 && !tile.model.isDir ? dot : text.length) }
        onAccepted: { tile.folder.rename(tile.index, text); tile.browser.renameRow = -1 }
        Keys.onEscapePressed: tile.browser.renameRow = -1
        onActiveFocusChanged: if (!activeFocus && visible) { tile.folder.rename(tile.index, text); tile.browser.renameRow = -1 }
    }

    MouseArea {
        id: ma
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onPressed: m => { if (m.button === Qt.LeftButton && !tile.selected) tile.browser.select(tile.index, m.modifiers) }
        onClicked: m => {
            if (m.button === Qt.RightButton) tile.browser.showItemMenu(tile.index)
            else if (tile.selected && !(m.modifiers & (Qt.ControlModifier | Qt.ShiftModifier))) tile.browser.select(tile.index, m.modifiers)
        }
        onDoubleClicked: tile.folder.open(tile.index)
        drag.target: dragProxy
    }
    Item {
        id: dragProxy
        Drag.active: ma.drag.active
        Drag.dragType: Drag.Automatic
        Drag.supportedActions: Qt.CopyAction | Qt.MoveAction
        Drag.mimeData: { "text/uri-list": tile.folder.urlsAt(tile.browser.rows()).map(u => u.toString()).join("\r\n") }
    }
    DropArea {
        id: drop
        anchors.fill: parent
        enabled: tile.model.isDir
        keys: ["text/uri-list"]
        onDropped: d => { tile.folder.copyUrls(d.urls, tile.model.url, !(d.modifiers & Qt.ControlModifier)); d.acceptProposedAction() }
    }
}
