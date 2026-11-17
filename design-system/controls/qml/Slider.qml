// SPDX-License-Identifier: GPL-3.0-or-later
// Linear slider with a capsule knob that swells while dragged.
import QtQuick
import QtQuick.Templates as T
import Lumen.Design
import Lumen.Motion

T.Slider {
    id: control
    implicitWidth: 200
    implicitHeight: 22

    Spring { id: press; to: control.pressed ? 1 : 0; preset: Spring.Snappy }
    Spring { id: vis; to: control.visualPosition; preset: Spring.Critical; enabled: !control.pressed }

    readonly property real _pos: control.pressed ? control.visualPosition : vis.value

    background: Rectangle {
        x: control.leftPadding
        y: control.topPadding + control.availableHeight / 2 - height / 2
        width: control.availableWidth
        height: 4
        radius: 2
        color: Theme.controlFillPressed
        Rectangle {
            width: Math.max(height, control._pos * parent.width)
            height: parent.height
            radius: 2
            color: Theme.accent
            opacity: control.enabled ? 1 : 0.4
        }
    }

    handle: Rectangle {
        readonly property real s: 1 + 0.15 * press.value
        x: control.leftPadding + control._pos * (control.availableWidth - width)
        y: control.topPadding + control.availableHeight / 2 - height / 2
        width: 22 * s; height: 15 * s
        radius: height / 2
        color: "white"
        border.width: Theme.dark ? 0 : 0.5
        border.color: Qt.rgba(0, 0, 0, 0.12)
        Rectangle {
            z: -1; anchors.fill: parent; anchors.topMargin: 1; anchors.bottomMargin: -1.5
            radius: parent.radius; color: Qt.rgba(0, 0, 0, 0.22)
        }
    }
}
