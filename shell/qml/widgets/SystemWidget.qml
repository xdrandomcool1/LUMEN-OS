// SPDX-License-Identifier: GPL-3.0-or-later
// CPU / memory / disk / network at a glance.
import QtQuick
import Lumen.Design
import Lumen.Shell

WidgetFrame {
    id: w
    medium: true
    SysStats { id: stats; active: w.visible }

    component Gauge: Item {
        property real value
        property string label
        property string detail
        property color tint: Theme.accent
        width: 70; height: 100
        Canvas {
            id: ring
            width: 56; height: 56
            anchors.horizontalCenter: parent.horizontalCenter
            property real v: parent.value
            onVChanged: requestPaint()
            onPaint: {
                const c = getContext("2d"); const r = width / 2 - 4
                c.reset(); c.lineWidth = 6; c.lineCap = "round"
                c.strokeStyle = Theme.dark ? "rgba(255,255,255,0.12)" : "rgba(0,0,0,0.08)"
                c.beginPath(); c.arc(width / 2, height / 2, r, 0, 2 * Math.PI); c.stroke()
                c.strokeStyle = parent.tint.toString()
                c.beginPath(); c.arc(width / 2, height / 2, r, -Math.PI / 2, -Math.PI / 2 + 2 * Math.PI * Math.max(0.001, v)); c.stroke()
            }
            LText { anchors.centerIn: parent; text: Math.round(parent.v * 100) + "%"; style: LText.Caption; font.weight: Font.DemiBold }
        }
        LText { anchors.horizontalCenter: parent.horizontalCenter; y: 62; text: parent.label; style: LText.Caption; font.weight: Font.DemiBold }
        LText { anchors.horizontalCenter: parent.horizontalCenter; y: 78; text: parent.detail; style: LText.Caption2; secondary: true }
    }

    Row {
        anchors.centerIn: parent
        spacing: 12
        Gauge { value: stats.cpu; label: qsTr("CPU"); tint: Theme.blue }
        Gauge { value: stats.memory; label: qsTr("Memory"); detail: stats.formatBytes(stats.memoryUsed); tint: Theme.purple }
        Gauge { value: stats.disk; label: qsTr("Disk"); tint: Theme.orange }
        Column {
            width: 70; topPadding: 12; spacing: 6
            LText { text: qsTr("Network"); style: LText.Caption; font.weight: Font.DemiBold }
            LText { text: "↓ " + stats.formatBytes(stats.netDown) + "/s"; style: LText.Caption2 }
            LText { text: "↑ " + stats.formatBytes(stats.netUp) + "/s"; style: LText.Caption2 }
        }
    }
    MouseArea { anchors.fill: parent; onClicked: AppsModel.launch("lumen-activity-monitor") }
}
