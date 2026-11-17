// SPDX-License-Identifier: GPL-3.0-or-later
// Close / minimize / zoom buttons. Glyphs appear when the group is hovered.
import QtQuick
import Lumen.Design
import Lumen.Motion

Row {
    id: root
    property bool active: true
    property bool closable: true
    property bool minimizable: true
    property bool zoomable: true
    property bool fullscreenGlyph: false
    signal closeRequested()
    signal minimizeRequested()
    signal zoomRequested()

    spacing: Theme.metrics.trafficSpacing - Theme.metrics.trafficLight
    readonly property bool groupHovered: hoverArea.containsMouse

    HoverHandler { id: hoverArea; property bool containsMouse: hovered }

    component Light: Item {
        id: light
        property color tint
        property string glyph
        property bool enabledLight: true
        signal clicked()
        width: Theme.metrics.trafficLight; height: width
        Spring { id: p; to: ma.pressed ? 1 : 0; preset: Spring.Snappy }
        Rectangle {
            anchors.fill: parent
            radius: width / 2
            color: !light.enabledLight ? Theme.inactiveButton
                   : (root.active || root.groupHovered) ? Qt.darker(light.tint, 1 + 0.18 * p.value) : Theme.inactiveButton
            border.width: 0.5
            border.color: Qt.rgba(0, 0, 0, Theme.dark ? 0.25 : 0.12)
        }
        Canvas {
            anchors.fill: parent
            visible: root.groupHovered && light.enabledLight
            property string g: light.glyph
            onGChanged: requestPaint()
            onPaint: {
                const c = getContext("2d"); const w = width
                c.reset(); c.strokeStyle = "rgba(0,0,0,0.55)"; c.fillStyle = "rgba(0,0,0,0.55)"; c.lineWidth = 1.2; c.lineCap = "round"
                c.beginPath()
                if (g === "close") { c.moveTo(w*0.32, w*0.32); c.lineTo(w*0.68, w*0.68); c.moveTo(w*0.68, w*0.32); c.lineTo(w*0.32, w*0.68); c.stroke() }
                else if (g === "min") { c.moveTo(w*0.27, w*0.5); c.lineTo(w*0.73, w*0.5); c.stroke() }
                else if (g === "zoom") {
                    // Two opposing triangles (diagonal arrows)
                    c.moveTo(w*0.30, w*0.30); c.lineTo(w*0.62, w*0.30); c.lineTo(w*0.30, w*0.62); c.closePath(); c.fill()
                    c.beginPath(); c.moveTo(w*0.70, w*0.70); c.lineTo(w*0.38, w*0.70); c.lineTo(w*0.70, w*0.38); c.closePath(); c.fill()
                }
            }
        }
        MouseArea { id: ma; anchors.fill: parent; anchors.margins: -2; enabled: light.enabledLight; onClicked: light.clicked() }
    }

    Light { tint: Theme.closeButton; glyph: "close"; enabledLight: root.closable; onClicked: root.closeRequested() }
    Light { tint: Theme.minimizeButton; glyph: "min"; enabledLight: root.minimizable; onClicked: root.minimizeRequested() }
    Light { tint: Theme.zoomButton; glyph: "zoom"; enabledLight: root.zoomable; onClicked: root.zoomRequested() }
}
