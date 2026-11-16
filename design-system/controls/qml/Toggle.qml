// SPDX-License-Identifier: GPL-3.0-or-later
// On/off switch. The knob stretches while pressed and travels on a spring.
import QtQuick
import QtQuick.Templates as T
import Lumen.Design
import Lumen.Motion

T.Switch {
    id: control
    property bool small: false
    readonly property real trackW: small ? 26 : 38
    readonly property real trackH: small ? 15 : 22

    implicitWidth: indicator.width + (text.length ? spacing + contentItem.implicitWidth : 0)
    implicitHeight: Math.max(trackH, contentItem.implicitHeight)
    spacing: 8

    Spring { id: pos; to: control.checked ? 1 : 0; preset: Spring.Snappy }
    Spring { id: press; to: control.down ? 1 : 0; preset: Spring.Snappy }

    indicator: Rectangle {
        x: control.text.length ? control.width - width : 0
        anchors.verticalCenter: parent.verticalCenter
        width: control.trackW; height: control.trackH
        radius: height / 2
        color: Qt.rgba(
            Theme.controlFillPressed.r + (Theme.accent.r - Theme.controlFillPressed.r) * pos.value,
            Theme.controlFillPressed.g + (Theme.accent.g - Theme.controlFillPressed.g) * pos.value,
            Theme.controlFillPressed.b + (Theme.accent.b - Theme.controlFillPressed.b) * pos.value,
            Theme.controlFillPressed.a + (1 - Theme.controlFillPressed.a) * pos.value)
        opacity: control.enabled ? 1 : 0.4

        Rectangle {
            id: knob
            readonly property real d: parent.height - 4
            readonly property real stretch: d * 0.28 * press.value
            width: d + stretch
            height: d
            radius: d / 2
            y: 2
            x: 2 + (parent.width - 4 - width) * pos.value
            color: "white"
            Rectangle { // shadow
                z: -1; anchors.fill: parent; anchors.topMargin: 1; anchors.bottomMargin: -1
                radius: parent.radius; color: Qt.rgba(0, 0, 0, 0.18)
            }
        }
    }

    contentItem: LText {
        text: control.text
        visible: text.length > 0
        rightPadding: control.indicator.width + control.spacing
    }
}
