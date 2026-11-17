// SPDX-License-Identifier: GPL-3.0-or-later
// Activity Monitor: processes + CPU / Memory / Disk / Network / GPU.
import QtQuick
import Lumen.Design
import Lumen.Motion
import Lumen.Controls
import Lumen.System
import Lumen.ActivityMonitor

AppWindow {
    id: win
    title: qsTr("Activity Monitor")
    subtitle: qsTr("%1 processes").arg(procs.processCount)
    width: 980; height: 620
    visible: true

    property int tab: 0
    readonly property var tabs: [qsTr("CPU"), qsTr("Memory"), qsTr("Disk"), qsTr("Network"), qsTr("GPU")]
    property int selectedPid: -1
    property string error

    ProcessModel { id: procs; active: win.visible; filter: search.text; sortKey: ["cpu", "memory", "read", "cpu", "cpu"][win.tab] }
    SysStats { id: stats; active: win.visible }
    GpuStats { id: gpu; active: win.visible && win.tab === 4 }

    toolbarLeading: [
        ToolbarGroup {
            ToolbarButton { symbol: "x-circle"; tooltip: qsTr("Quit Process"); enabled: win.selectedPid > 0; onClicked: confirm.visible = true }
            ToolbarButton { symbol: "info"; tooltip: qsTr("Inspect"); enabled: win.selectedPid > 0; onClicked: info.visible = !info.visible }
        }
    ]
    toolbarTrailing: [
        SegmentedControl { width: 340; model: win.tabs; currentIndex: win.tab; onActivated: i => win.tab = i },
        SearchField { id: search; width: 170; height: 30 }
    ]

    readonly property var columns: {
        const base = [{ k: "name", t: qsTr("Process Name"), w: 0.34 }]
        if (tab === 1) return base.concat([{ k: "memory", t: qsTr("Memory"), w: 0.16 }, { k: "threads", t: qsTr("Threads"), w: 0.12 }, { k: "pid", t: "PID", w: 0.12 }, { k: "user", t: qsTr("User"), w: 0.26 }])
        if (tab === 2) return base.concat([{ k: "read", t: qsTr("Bytes Read/s"), w: 0.2 }, { k: "write", t: qsTr("Bytes Written/s"), w: 0.2 }, { k: "pid", t: "PID", w: 0.1 }, { k: "user", t: qsTr("User"), w: 0.16 }])
        return base.concat([{ k: "cpu", t: qsTr("% CPU"), w: 0.12 }, { k: "threads", t: qsTr("Threads"), w: 0.12 }, { k: "memory", t: qsTr("Memory"), w: 0.14 }, { k: "pid", t: "PID", w: 0.1 }, { k: "user", t: qsTr("User"), w: 0.18 }])
    }
    function cell(m, k) {
        switch (k) {
        case "name": return m.name
        case "cpu": return m.cpu.toFixed(1)
        case "memory": return m.memoryText
        case "threads": return m.threads
        case "pid": return m.pid
        case "user": return m.user
        case "read": return stats.formatBytes(m.readRate)
        case "write": return stats.formatBytes(m.writeRate)
        }
        return ""
    }

    Item {
        x: 0; y: win.toolbarHeight
        width: win.contentArea.width; height: win.contentArea.height - y - summary.height
        visible: win.tab !== 3 && win.tab !== 4 || true
        Row {
            id: header
            x: 12; width: parent.width - 24; height: 26
            Repeater {
                model: win.columns
                Item {
                    required property var modelData
                    width: header.width * modelData.w; height: header.height
                    LText { anchors.verticalCenter: parent.verticalCenter; leftPadding: 8; style: LText.Callout; font.weight: Font.DemiBold; secondary: true
                            text: modelData.t + (procs.sortKey === modelData.k ? (procs.descending ? "  ▼" : "  ▲") : "") }
                    MouseArea { anchors.fill: parent; onClicked: { if (procs.sortKey === modelData.k) procs.descending = !procs.descending; else { procs.sortKey = modelData.k; procs.descending = modelData.k !== "name" && modelData.k !== "user" } } }
                }
            }
        }
        ListView {
            id: list
            anchors.top: header.bottom; anchors.bottom: parent.bottom
            width: parent.width
            clip: true
            model: procs
            ScrollBar.vertical: ScrollBar {}
            delegate: Item {
                id: row
                required property var model
                required property int index
                width: list.width; height: 24
                readonly property bool sel: row.model.pid === win.selectedPid
                Rectangle { anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; radius: 6
                            color: row.sel ? Theme.selection : (row.index % 2 ? Theme.controlFill : "transparent") }
                Row {
                    x: 12; height: parent.height
                    Repeater {
                        model: win.columns
                        LText {
                            required property var modelData
                            width: header.width * modelData.w; anchors.verticalCenter: parent.verticalCenter
                            leftPadding: 8; text: win.cell(row.model, modelData.k)
                            color: row.sel ? "white" : Theme.label
                            font.features: { "tnum": 1 }
                        }
                    }
                }
                MouseArea { anchors.fill: parent; onClicked: win.selectedPid = row.model.pid; onDoubleClicked: { win.selectedPid = row.model.pid; info.visible = true } }
            }
        }
    }

    // ---- Summary panel ---------------------------------------------------
    Rectangle {
        id: summary
        anchors.bottom: parent.bottom
        x: win.contentArea.x; width: win.contentArea.width; height: 150
        color: Theme.dark ? Qt.rgba(1, 1, 1, 0.03) : Qt.rgba(0, 0, 0, 0.025)
        Rectangle { width: parent.width; height: 1; color: Theme.separator }
        Row {
            anchors.fill: parent; anchors.margins: 16; spacing: 24
            Column {
                width: 260; spacing: 6
                LText { style: LText.Headline; text: [qsTr("CPU"), qsTr("Memory"), qsTr("Disk"), qsTr("Network"), qsTr("GPU")][win.tab] }
                Repeater {
                    model: {
                        if (win.tab === 1) return [[qsTr("Memory used"), stats.formatBytes(stats.memoryUsed)], [qsTr("Physical memory"), stats.formatBytes(stats.memoryTotal)], [qsTr("Pressure"), Math.round(stats.memory * 100) + "%"]]
                        if (win.tab === 2) return [[qsTr("Disk used"), Math.round(stats.disk * 100) + "%"]]
                        if (win.tab === 3) return [[qsTr("Received/s"), stats.formatBytes(stats.netDown)], [qsTr("Sent/s"), stats.formatBytes(stats.netUp)]]
                        if (win.tab === 4) return gpu.available ? [[qsTr("Utilisation"), Math.round(gpu.usage * 100) + "%"], [qsTr("Source"), gpu.source]] : [[qsTr("GPU statistics"), qsTr("Not reported by this driver")]]
                        return [[qsTr("System load"), Math.round(stats.cpu * 100) + "%"], [qsTr("Processes"), procs.processCount], [qsTr("Threads"), procs.threadCount]]
                    }
                    Row { required property var modelData; spacing: 8
                        LText { width: 130; text: modelData[0]; secondary: true; style: LText.Callout }
                        LText { text: modelData[1]; style: LText.Callout; font.weight: Font.DemiBold } }
                }
            }
            Canvas {
                id: chart
                width: parent.width - 300; height: parent.height
                property var history: []
                Connections { target: stats; function onUpdated() {
                    const v = win.tab === 1 ? stats.memory : win.tab === 2 ? stats.disk : win.tab === 4 ? gpu.usage : win.tab === 3 ? Math.min(1, stats.netDown / 1e6) : stats.cpu
                    chart.history = chart.history.concat([v]).slice(-60); chart.requestPaint() } }
                onPaint: {
                    const c = getContext("2d"); c.reset()
                    c.fillStyle = Theme.dark ? "rgba(255,255,255,0.04)" : "rgba(0,0,0,0.04)"; c.fillRect(0, 0, width, height)
                    if (history.length < 2) return
                    const step = width / 59
                    const col = [Theme.blue, Theme.purple, Theme.orange, Theme.teal, Theme.green][win.tab]
                    c.beginPath(); c.moveTo(width - (history.length - 1) * step, height)
                    for (let i = 0; i < history.length; ++i) c.lineTo(width - (history.length - 1 - i) * step, height - history[i] * height)
                    c.lineTo(width, height); c.closePath()
                    c.fillStyle = Qt.rgba(col.r, col.g, col.b, 0.25).toString(); c.fill()
                    c.beginPath()
                    for (let i = 0; i < history.length; ++i) { const x = width - (history.length - 1 - i) * step, y = height - history[i] * height; i ? c.lineTo(x, y) : c.moveTo(x, y) }
                    c.strokeStyle = col.toString(); c.lineWidth = 2; c.stroke()
                }
            }
        }
    }

    // ---- Quit confirmation / inspector ------------------------------------
    Rectangle {
        id: confirm
        visible: false
        anchors.centerIn: parent
        width: 360; height: 150; radius: 16
        color: Theme.dark ? "#2c2c2e" : "white"; border.width: 0.5; border.color: Theme.controlBorder
        Column {
            anchors.fill: parent; anchors.margins: 16; spacing: 12
            LText { text: qsTr("Are you sure you want to quit this process?"); style: LText.Headline; width: parent.width; wrapMode: Text.Wrap }
            LText { text: (procs.details(win.selectedPid).name || "") + " (" + win.selectedPid + ")"; secondary: true }
            LText { visible: win.error.length > 0; text: win.error; color: Theme.red; style: LText.Callout; width: parent.width; wrapMode: Text.Wrap }
            Row { anchors.right: parent.right; spacing: 8
                Button { text: qsTr("Cancel"); onClicked: { confirm.visible = false; win.error = "" } }
                Button { text: qsTr("Force Quit"); kind: Button.Destructive; onClicked: { win.error = procs.sendSignal(win.selectedPid, true); if (!win.error) confirm.visible = false } }
                Button { text: qsTr("Quit"); kind: Button.Prominent; onClicked: { win.error = procs.sendSignal(win.selectedPid, false); if (!win.error) confirm.visible = false } } }
        }
    }
    Rectangle {
        id: info
        visible: false
        anchors.right: parent.right; anchors.rightMargin: 16; y: win.toolbarHeight + 10
        width: 320; height: infoCol.implicitHeight + 28; radius: 14
        color: Theme.dark ? "#2c2c2e" : "white"; border.width: 0.5; border.color: Theme.controlBorder
        readonly property var d: win.selectedPid > 0 ? procs.details(win.selectedPid) : ({})
        Column {
            id: infoCol
            x: 14; y: 14; width: parent.width - 28; spacing: 6
            LText { text: info.d.name || ""; style: LText.Title3 }
            LText { text: qsTr("PID %1 · parent %2 · %3").arg(info.d.pid).arg(info.d.ppid).arg(info.d.user); secondary: true; style: LText.Callout }
            LText { width: parent.width; text: info.d.command || ""; style: LText.Caption; wrapMode: Text.WrapAnywhere; font.family: Theme.monoFamily }
        }
    }
}
