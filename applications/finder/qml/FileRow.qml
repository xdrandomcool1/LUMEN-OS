// SPDX-License-Identifier: GPL-3.0-or-later
// List-view row.
import QtQuick
import Lumen.Design
import Lumen.Controls

Item {
    id: row
    required property int index
    required property var model
    property var browser
    property var folder
    readonly property bool selected: browser.selection[index] === true
    height: 26
    readonly property real wName: width * 0.46
    readonly property real wDate: width * 0.24
    readonly property real wSize: width * 0.12

    Rectangle {
        anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8
        radius: 6
        color: row.selected ? Theme.selection : (drop.containsDrag ? Theme.controlFillPressed : "transparent")
    }
    readonly property color fg: selected ? "white" : Theme.label
    readonly property color fg2: selected ? Qt.rgba(1, 1, 1, 0.8) : Theme.secondaryLabel
    FileIcon { x: 18; anchors.verticalCenter: parent.verticalCenter; size: 18; iconName: row.model.iconName; url: row.model.url; isImage: false }
    LText { x: 44; width: row.wName - 50; anchors.verticalCenter: parent.verticalCenter; text: row.model.name; color: row.fg; visible: row.browser.renameRow !== row.index; elide: Text.ElideMiddle }
    TextField {
        visible: row.browser.renameRow === row.index
        x: 40; width: row.wName - 46; height: 22; anchors.verticalCenter: parent.verticalCenter
        onVisibleChanged: if (visible) { text = row.model.name; forceActiveFocus(); selectAll() }
        onAccepted: { row.folder.rename(row.index, text); row.browser.renameRow = -1 }
        Keys.onEscapePressed: row.browser.renameRow = -1
    }
    LText { x: row.wName; width: row.wDate - 10; anchors.verticalCenter: parent.verticalCenter; text: row.model.modifiedText; color: row.fg2 }
    LText { x: row.wName + row.wDate; width: row.wSize - 10; anchors.verticalCenter: parent.verticalCenter; text: row.model.sizeText; color: row.fg2; horizontalAlignment: Text.AlignRight }
    LText { x: row.wName + row.wDate + row.wSize + 14; width: row.width - x - 16; anchors.verticalCenter: parent.verticalCenter; text: row.model.kind; color: row.fg2 }

    MouseArea {
        id: ma
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onPressed: m => { if (m.button === Qt.LeftButton && !row.selected) row.browser.select(row.index, m.modifiers) }
        onClicked: m => { if (m.button === Qt.RightButton) row.browser.showItemMenu(row.index); else if (row.selected) row.browser.select(row.index, m.modifiers) }
        onDoubleClicked: row.folder.open(row.index)
        drag.target: dragProxy
    }
    Item {
        id: dragProxy
        Drag.active: ma.drag.active
        Drag.dragType: Drag.Automatic
        Drag.supportedActions: Qt.CopyAction | Qt.MoveAction
        Drag.mimeData: { "text/uri-list": row.folder.urlsAt(row.browser.rows()).map(u => u.toString()).join("\r\n") }
    }
    DropArea {
        id: drop
        anchors.fill: parent
        enabled: row.model.isDir
        keys: ["text/uri-list"]
        onDropped: d => { row.folder.copyUrls(d.urls, row.model.url, !(d.modifiers & Qt.ControlModifier)); d.acceptProposedAction() }
    }
}
