// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Templates as T
import Lumen.Design
import Lumen.Motion

T.CheckBox {
    id: control
    implicitWidth: indicator.width + (text.length ? spacing + contentItem.implicitWidth : 0)
    implicitHeight: Math.max(16, contentItem.implicitHeight)
    spacing: 6

    Spring { id: on; to: control.checkState !== Qt.Unchecked ? 1 : 0; preset: Spring.Snappy }

    indicator: Rectangle {
        width: 14; height: 14; radius: 4
        anchors.verticalCenter: parent.verticalCenter
        color: on.value > 0.5 ? Theme.accent : Theme.controlBackground
        border.width: on.value > 0.5 ? 0 : 0.5
        border.color: Theme.controlBorder
        opacity: control.enabled ? 1 : 0.4
        Symbol {
            anchors.centerIn: parent
            width: 11; height: 11
            weight: Symbol.Bold
            name: control.checkState === Qt.PartiallyChecked ? "minus" : "check"
            color: "white"
            opacity: on.value
            scale: 0.6 + 0.4 * on.value
        }
        Rectangle {
            anchors.fill: parent; anchors.margins: -3; radius: 6
            color: "transparent"; border.width: 3; border.color: Theme.focusRing
            visible: control.visualFocus
        }
    }
    contentItem: LText {
        text: control.text
        leftPadding: control.indicator.width + control.spacing
    }
}
