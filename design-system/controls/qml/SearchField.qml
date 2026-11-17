// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import Lumen.Design

TextField {
    id: field
    capsule: true
    placeholderText: qsTr("Search")
    leftPadding: 28
    rightPadding: text.length ? 26 : 12
    Symbol {
        x: 9; anchors.verticalCenter: parent.verticalCenter
        width: 14; height: 14; name: "magnifying-glass"; color: Theme.secondaryLabel
    }
    Symbol {
        visible: field.text.length > 0
        anchors.right: parent.right; anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        width: 14; height: 14; name: "x-circle"; weight: Symbol.Fill; color: Theme.tertiaryLabel
        MouseArea { anchors.fill: parent; anchors.margins: -4; onClicked: field.clear() }
    }
}
