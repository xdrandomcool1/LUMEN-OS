// SPDX-License-Identifier: GPL-3.0-or-later
// Thick capsule slider with an embedded glyph (Display, Sound).
import QtQuick
import Lumen.Design
import Lumen.Motion

Item {
    id: s
    property real value: 0
    property string symbol: "sun"
    property bool enabledSlider: true
    signal moved(real value)
    implicitHeight: 30
    implicitWidth: 260

    Spring { id: shown; to: s.value; preset: Spring.Critical; enabled: !drag.pressed }
    readonly property real vis: drag.pressed ? s.value : shown.value
    Spring { id: grow; to: drag.pressed ? 1 : 0; preset: Spring.Snappy }

    Rectangle {
        id: track
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width
        height: s.height * (1 + 0.08 * grow.value)
        radius: height / 2
        color: Theme.dark ? Qt.rgba(1, 1, 1, 0.14) : Qt.rgba(0, 0, 0, 0.08)
        clip: true
        opacity: s.enabledSlider ? 1 : 0.45
        Rectangle {
            width: Math.max(track.height, track.height / 2 + s.vis * (track.width - track.height / 2))
            height: track.height
            radius: track.radius
            color: Qt.rgba(1, 1, 1, Theme.dark ? 0.92 : 1)
        }
        Symbol {
            x: 8; anchors.verticalCenter: parent.verticalCenter
            width: 16; height: 16; name: s.symbol; weight: Symbol.Fill
            color: Qt.rgba(0.3, 0.3, 0.32, 1)
        }
    }
    MouseArea {
        id: drag
        anchors.fill: parent
        enabled: s.enabledSlider
        preventStealing: true
        function update(x) { const v = Math.max(0, Math.min(1, x / width)); s.value = v; s.moved(v) }
        onPressed: e => update(e.x)
        onPositionChanged: e => { if (pressed) update(e.x) }
        onWheel: w => { const v = Math.max(0, Math.min(1, s.value + (w.angleDelta.y > 0 ? 0.05 : -0.05))); s.value = v; s.moved(v) }
    }
}
