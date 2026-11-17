// SPDX-License-Identifier: GPL-3.0-or-later
// Activity indicator: 12 fading spokes.
import QtQuick
import Lumen.Design

Item {
    id: root
    property bool running: true
    property color color: Theme.secondaryLabel
    implicitWidth: 18; implicitHeight: 18
    visible: running
    property int phase: 0
    Timer { interval: 83; repeat: true; running: root.running && root.visible; onTriggered: root.phase = (root.phase + 1) % 12 }
    Repeater {
        model: 12
        Rectangle {
            required property int index
            width: Math.max(1.5, root.width * 0.09); height: root.height * 0.28
            radius: width / 2
            x: root.width / 2 - width / 2; y: 0
            color: root.color
            opacity: 0.18 + 0.82 * (((index - root.phase + 12) % 12) / 11)
            transform: Rotation { origin.x: width / 2; origin.y: root.height / 2; angle: index * 30 }
        }
    }
}
