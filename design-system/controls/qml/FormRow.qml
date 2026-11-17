// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import Lumen.Design

Item {
    id: row
    property string label: ""
    property string detail: ""
    property string symbol: ""
    property color symbolTint: Theme.accent
    property bool separator: true
    property bool clickable: false
    default property alias trailing: trailingItem.data
    signal clicked()
    width: parent ? parent.width : 400
    implicitHeight: detail.length ? 48 : 38

    Rectangle {
        anchors.fill: parent; anchors.margins: 2; radius: 8
        color: Theme.controlFill
        visible: row.clickable && ma.containsMouse
    }
    Row {
        anchors.left: parent.left; anchors.leftMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        spacing: 9
        Rectangle {
            visible: row.symbol.length > 0
            width: 22; height: 22; radius: 6
            color: row.symbolTint
            anchors.verticalCenter: parent.verticalCenter
            Symbol { anchors.centerIn: parent; width: 14; height: 14; name: row.symbol; weight: Symbol.Fill; color: "white" }
        }
        Column {
            anchors.verticalCenter: parent.verticalCenter
            LText { text: row.label }
            LText { visible: row.detail.length > 0; text: row.detail; style: LText.Callout; secondary: true }
        }
    }
    Item {
        id: trailingItem
        anchors.right: parent.right; anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        width: childrenRect.width; height: childrenRect.height
    }
    Rectangle {
        visible: row.separator
        anchors.bottom: parent.bottom
        x: row.symbol.length ? 43 : 12; width: parent.width - x - 12; height: 1
        color: Theme.separator
    }
    MouseArea { id: ma; anchors.fill: parent; enabled: row.clickable; hoverEnabled: true; onClicked: row.clicked(); z: -1 }
}
