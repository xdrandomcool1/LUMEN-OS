// SPDX-License-Identifier: GPL-3.0-or-later
// Scrollable settings pane with a centred column of grouped sections.
import QtQuick
import Lumen.Design
import Lumen.Controls

Flickable {
    id: p
    default property alias content: col.data
    property string heading
    property string symbol
    property color tint: Theme.accent
    property string summary
    contentHeight: col.implicitHeight + 40
    clip: true
    boundsBehavior: Flickable.DragAndOvershootBounds
    ScrollBar.vertical: ScrollBar {}
    Column {
        id: col
        width: Math.min(p.width - 48, 620)
        x: (p.width - width) / 2
        y: 8
        spacing: 22
        Column {
            visible: p.summary.length > 0
            width: parent.width
            spacing: 8
            Rectangle { anchors.horizontalCenter: parent.horizontalCenter; width: 56; height: 56; radius: 14; color: p.tint
                Symbol { anchors.centerIn: parent; width: 32; height: 32; name: p.symbol; weight: Symbol.Fill; color: "white" } }
            LText { anchors.horizontalCenter: parent.horizontalCenter; text: p.heading; style: LText.Title2 }
            LText { width: parent.width; text: p.summary; secondary: true; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.Wrap; style: LText.Callout }
        }
    }
}
