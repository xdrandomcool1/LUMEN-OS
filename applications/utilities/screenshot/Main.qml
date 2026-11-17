// SPDX-License-Identifier: GPL-3.0-or-later
// Screenshot toolbar: capture screen, window or a selected area.
import QtQuick
import QtQuick.Window
import Lumen.Design
import Lumen.Motion
import Lumen.Glass
import Lumen.Controls
import Lumen.Screenshot

Window {
    id: bar
    width: 520; height: 64
    visible: true
    color: "transparent"
    flags: Qt.Window | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    title: qsTr("Screenshot")
    property int delay: 0

    Screenshooter {
        id: shot
        onCaptured: path => { thumb.source = "file://" + path; thumbTimer.restart(); bar.visible = true }
        onFrozenReady: { selector.showFullScreen() }
        onFailed: { errorText.text = shot.error; bar.visible = true }
    }
    WindowEffects { window: bar; radius: Theme.metrics.widgetRadius; blurBehind: true }

    GlassSurface {
        anchors.fill: parent; anchors.margins: 6
        radius: Theme.metrics.widgetRadius; material: Glass.Regular; compositorBlur: true
        Row {
            anchors.centerIn: parent; spacing: 6
            ToolbarButton { symbol: "x-circle"; tooltip: qsTr("Close"); onClicked: Qt.quit() }
            Separator { vertical: true; height: 24; anchors.verticalCenter: parent.verticalCenter }
            ToolbarButton { symbol: "monitor"; tooltip: qsTr("Capture Entire Screen"); onClicked: { bar.visible = false; shot.capture("screen", bar.delay * 1000) } }
            ToolbarButton { symbol: "app-window"; tooltip: qsTr("Capture Selected Window"); onClicked: { bar.visible = false; shot.capture("window", Math.max(bar.delay * 1000, 400)) } }
            ToolbarButton { symbol: "selection"; tooltip: qsTr("Capture Selected Portion"); onClicked: { bar.visible = false; shot.capture("freeze", bar.delay * 1000) } }
            Separator { vertical: true; height: 24; anchors.verticalCenter: parent.verticalCenter }
            PopUpButton { width: 120; model: [qsTr("No Timer"), qsTr("5 Seconds"), qsTr("10 Seconds")]; onActivated: i => bar.delay = [0, 5, 10][i]; anchors.verticalCenter: parent.verticalCenter }
            LText { id: errorText; color: Theme.red; style: LText.Caption; anchors.verticalCenter: parent.verticalCenter }
        }
    }

    // Floating thumbnail of the last capture (click to open).
    Window {
        id: thumbWin
        visible: thumb.status === Image.Ready && thumbTimer.running
        width: 220; height: 140; color: "transparent"
        flags: Qt.Window | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
        Rectangle { anchors.fill: parent; radius: Theme.metrics.hoverRadius; color: Theme.windowBackground; border.width: 1; border.color: Theme.controlBorder; clip: true
            Image { id: thumb; anchors.fill: parent; anchors.margins: 4; fillMode: Image.PreserveAspectFit; asynchronous: true }
            MouseArea { anchors.fill: parent; onClicked: shot.openLast() } }
        Timer { id: thumbTimer; interval: 5000 }
    }

    // Area selection over the frozen screen image.
    Window {
        id: selector
        visible: false
        color: "black"
        flags: Qt.Window | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
        Image { anchors.fill: parent; source: selector.visible ? "image://frozen/now" : ""; cache: false }
        Rectangle { anchors.fill: parent; color: Qt.rgba(0, 0, 0, 0.35) }
        property point a; property point b; property bool dragging: false
        readonly property rect sel: Qt.rect(Math.min(a.x, b.x), Math.min(a.y, b.y), Math.abs(a.x - b.x), Math.abs(a.y - b.y))
        Rectangle { visible: selector.dragging || selector.sel.width > 0; x: selector.sel.x; y: selector.sel.y; width: selector.sel.width; height: selector.sel.height
                    color: Qt.rgba(1, 1, 1, 0.08); border.width: 1; border.color: "white" }
        LText { visible: selector.dragging; x: selector.sel.x + selector.sel.width + 6; y: selector.sel.y + selector.sel.height + 6
                text: Math.round(selector.sel.width) + " × " + Math.round(selector.sel.height); color: "white" }
        MouseArea {
            anchors.fill: parent; cursorShape: Qt.CrossCursor
            onPressed: m => { selector.a = Qt.point(m.x, m.y); selector.b = selector.a; selector.dragging = true }
            onPositionChanged: m => selector.b = Qt.point(m.x, m.y)
            onReleased: { selector.dragging = false; selector.close(); if (selector.sel.width > 3 && selector.sel.height > 3) shot.saveCrop(selector.sel); else bar.visible = true }
        }
        Shortcut { sequence: "Escape"; onActivated: { selector.close(); bar.visible = true } }
    }
}
