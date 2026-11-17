// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Templates as T
import Lumen.Design

T.TextField {
    id: control
    property bool capsule: false
    implicitWidth: 200
    implicitHeight: 24
    leftPadding: capsule ? 12 : 7
    rightPadding: leftPadding
    verticalAlignment: TextInput.AlignVCenter
    color: Theme.label
    placeholderTextColor: Theme.tertiaryLabel
    selectionColor: Theme.selection
    selectedTextColor: "white"
    font.family: Theme.fontFamily
    font.pixelSize: 13

    Text {
        x: control.leftPadding; y: control.topPadding
        width: control.availableWidth; height: control.availableHeight
        text: control.placeholderText
        font: control.font
        color: control.placeholderTextColor
        verticalAlignment: control.verticalAlignment
        visible: !control.length && !control.preeditText
        renderType: Text.QtRendering
        elide: Text.ElideRight
    }

    background: Rectangle {
        radius: control.capsule ? height / 2 : 7
        color: Theme.dark ? Qt.rgba(1, 1, 1, 0.07) : "white"
        border.width: 0.5
        border.color: Theme.controlBorder
        Rectangle {
            anchors.fill: parent; anchors.margins: -3.5
            radius: parent.radius + 3.5
            color: "transparent"
            border.width: 3.5
            border.color: Theme.focusRing
            opacity: control.activeFocus ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: 120 } }
        }
    }
}
