// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Templates as T
import Lumen.Design
import Lumen.Motion

T.ProgressBar {
    id: control
    implicitWidth: 200
    implicitHeight: 6
    Spring { id: v; to: control.visualPosition; preset: Spring.Smooth }
    background: Rectangle { radius: 3; color: Theme.controlFillPressed }
    contentItem: Item {
        clip: true
        Rectangle {
            visible: !control.indeterminate
            width: Math.max(height, v.value * parent.width); height: parent.height
            radius: 3; color: Theme.accent
        }
        Rectangle {
            id: bar
            visible: control.indeterminate
            width: parent.width * 0.3; height: parent.height; radius: 3
            color: Theme.accent
            NumberAnimation on x {
                running: control.indeterminate && control.visible
                from: -bar.width; to: bar.parent.width; duration: 1100; loops: Animation.Infinite
                easing.type: Easing.InOutQuad
            }
        }
    }
}
