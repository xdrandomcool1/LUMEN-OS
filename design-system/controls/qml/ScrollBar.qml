// SPDX-License-Identifier: GPL-3.0-or-later
// Overlay scroller: appears while scrolling or hovered, then fades away.
import QtQuick
import QtQuick.Templates as T
import Lumen.Design
import Lumen.Motion

T.ScrollBar {
    id: control
    implicitWidth: orientation === Qt.Vertical ? 11 : 100
    implicitHeight: orientation === Qt.Horizontal ? 11 : 100
    padding: 2
    minimumSize: 0.08
    visible: policy !== T.ScrollBar.AlwaysOff && size < 1.0

    Spring { id: shown; to: (control.active || control.hovered || control.pressed) ? 1 : 0; preset: Spring.Smooth }

    contentItem: Rectangle {
        implicitWidth: control.hovered || control.pressed ? 7 : 5
        implicitHeight: implicitWidth
        radius: width / 2
        color: Theme.dark ? Qt.rgba(1, 1, 1, 0.45) : Qt.rgba(0, 0, 0, 0.42)
        opacity: shown.value
    }
}
