// SPDX-License-Identifier: GPL-3.0-or-later
// Sortable column header for the list view.
import QtQuick
import Lumen.Design
import Lumen.Controls

Rectangle {
    id: h
    property var folder
    height: 28
    color: Theme.contentBackground
    component Col: Item {
        property string key
        property string title
        property bool alignRight: false
        height: h.height
        LText {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.alignRight ? undefined : parent.left
            anchors.right: parent.alignRight ? parent.right : undefined
            anchors.rightMargin: 10
            text: parent.title + (h.folder.sortKey === parent.key ? (h.folder.ascending ? "  ▲" : "  ▼") : "")
            style: LText.Callout; font.weight: Font.DemiBold; secondary: true
        }
        MouseArea { anchors.fill: parent; onClicked: { if (h.folder.sortKey === parent.key) h.folder.ascending = !h.folder.ascending; else h.folder.sortKey = parent.key } }
    }
    Col { x: 44; width: h.width * 0.46 - 44; key: "name"; title: qsTr("Name") }
    Col { x: h.width * 0.46; width: h.width * 0.24; key: "modified"; title: qsTr("Date Modified") }
    Col { x: h.width * 0.70; width: h.width * 0.12; key: "size"; title: qsTr("Size"); alignRight: true }
    Col { x: h.width * 0.82 + 14; width: h.width * 0.18 - 14; key: "kind"; title: qsTr("Kind") }
    Rectangle { anchors.bottom: parent.bottom; x: 8; width: parent.width - 16; height: 1; color: Theme.separator }
}
