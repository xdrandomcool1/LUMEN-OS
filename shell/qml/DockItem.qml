// SPDX-License-Identifier: GPL-3.0-or-later
// One application tile in the Dock.
import QtQuick
import QtQuick.Effects
import Lumen.Design
import Lumen.Motion
import Lumen.Glass
import Lumen.Shell
import Lumen.Shell.Plasma

Item {
    id: item
    required property int index
    required property var model
    property var dock
    property var tasks

    readonly property real size: dock.sizeFor(index)
    readonly property bool running: model.IsWindow === true || model.IsStartup === true
    readonly property bool hovered: hoverArea.containsMouse
    property bool launching: false

    width: size + dock.spacing
    height: 1  // the Row baseline; icons grow upwards

    // Launch bounce: repeats while the app is starting (startup notification).
    readonly property bool bouncing: (launching || model.IsStartup === true) && model.IsWindow !== true
    onBouncingChanged: if (bouncing && !Motion.reduceMotion) bounce.restart()
    Timer { interval: 12000; running: item.launching; onTriggered: item.launching = false }
    property real bounceY: 0
    SequentialAnimation {
        id: bounce
        loops: Animation.Infinite
        NumberAnimation { target: item; property: "bounceY"; to: -item.dock.base * 0.42; duration: 260; easing.type: Easing.OutQuad }
        NumberAnimation { target: item; property: "bounceY"; to: 0; duration: 360; easing.type: Easing.OutBounce }
        PauseAnimation { duration: 80 }
        onStopped: item.bounceY = 0
    }
    onRunningChanged: if (model.IsWindow) { launching = false; if (!bouncing) { bounce.stop(); bounceY = 0 } }
    Connections { target: item; function onBouncingChanged() { if (!item.bouncing) bounce.loops = 1 } }

    Spring { id: press; to: mouse.pressed ? 1 : 0; preset: Spring.Snappy }

    AppIcon {
        id: icon
        size: item.size
        anchors.horizontalCenter: parent.horizontalCenter
        y: -size + item.bounceY
        source: item.model.decoration
        layer.enabled: press.value > 0.01
        layer.effect: MultiEffect { brightness: -0.35 * press.value }
    }

    // Running indicator
    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        y: item.dock.pad * 0.5 - 2
        width: 4; height: 4; radius: 2
        color: Theme.dark || WallpaperEngine.luminance < 0.5 ? Qt.rgba(1, 1, 1, 0.85) : Qt.rgba(0, 0, 0, 0.7)
        opacity: item.model.IsWindow === true ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: Motion.normal } }
    }
    // Attention badge
    Rectangle {
        visible: item.model.IsDemandingAttention === true
        x: icon.x + icon.width - width * 0.8; y: icon.y - height * 0.2
        width: 14; height: 14; radius: 7; color: Theme.red
    }

    // App name label above the icon while hovered.
    Item {
        id: label
        readonly property bool shown: item.hovered && !item.dock.dragging && ShellController.activePanel === ""
        anchors.horizontalCenter: parent.horizontalCenter
        y: icon.y - height - 10
        width: labelText.implicitWidth + 22
        height: 28
        opacity: labelFade.value
        visible: opacity > 0.01
        Spring { id: labelFade; to: label.shown ? 1 : 0; preset: Spring.Snappy }
        GlassSurface {
            anchors.fill: parent
            radius: height / 2
            material: Glass.Regular
            backdrop: null
            shadowScale: 0.4
            shadowOffsetY: 2
        }
        LText { id: labelText; anchors.centerIn: parent; text: item.model.AppName || item.model.display || ""; style: LText.Callout; font.weight: Font.Medium }
    }

    // Publish the icon geometry so KWin can animate minimize into it.
    function publishGeometry() {
        if (!item.dock.targetScreen || !item.model.IsWindow) return
        const p = icon.mapToItem(null, 0, 0)
        const scr = item.dock.targetScreen
        const r = Qt.rect(scr.virtualX + p.x, scr.virtualY + scr.height - item.dock.height + p.y + (item.size - item.dock.base),
                          item.dock.base, item.dock.base)
        tasks.requestPublishDelegateGeometry(tasks.makeModelIndex(item.index), r, item)
    }
    Timer { id: publishTimer; interval: 200; onTriggered: item.publishGeometry() }
    onXChanged: if (hover_value() < 0.01) publishTimer.restart()
    function hover_value() { return item.size === item.dock.base ? 0 : 1 }
    Component.onCompleted: publishTimer.restart()

    MouseArea {
        id: mouse
        x: (parent.width - item.size) / 2 - item.dock.spacing / 2
        y: -item.size - 4
        width: item.size + item.dock.spacing
        height: item.size + item.dock.pad + 4
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: true
        property alias containsMouseAlias: mouse.containsMouse
        onClicked: event => {
            const idx = tasks.makeModelIndex(item.index)
            if (event.button === Qt.RightButton) { item.openMenu(); return }
            if (item.model.IsLauncher === true && item.model.IsWindow !== true) {
                item.launching = true
                tasks.requestActivate(idx)
            } else if (event.modifiers & Qt.AltModifier) {
                tasks.requestToggleMinimized(idx)
            } else {
                tasks.requestActivate(idx)
            }
        }
        onPressAndHold: item.openMenu()

        // Drag to reorder (launchers and running apps).
        drag.target: null
        property real pressX: 0
        property bool reordering: false
        onPressed: event => { pressX = event.x; reordering = false }
        onPositionChanged: event => {
            if (!pressed) return
            const dx = event.x - pressX
            if (!reordering && Math.abs(dx) > 10) { reordering = true; item.dock.dragging = true }
            if (reordering) {
                const target = Math.max(0, Math.min(item.dock.appCount - 1, item.index + Math.round(dx / item.dock.tile)))
                if (target !== item.index) {
                    tasks.move(item.index, target)
                    pressX += (target - item.index) * item.dock.tile
                }
            }
        }
        onReleased: { if (reordering) { tasks.syncLaunchers(); item.dock.dragging = false; reordering = false } }
    }
    readonly property alias hoverArea: mouse

    DropArea {
        anchors.fill: mouse
        onDropped: drop => {
            if (drop.hasUrls) tasks.requestOpenUrls(tasks.makeModelIndex(item.index), drop.urls)
        }
    }

    function openMenu() {
        const idx = tasks.makeModelIndex(item.index)
        const url = item.model.LauncherUrlWithoutIcon ? item.model.LauncherUrlWithoutIcon.toString() : ""
        const pinned = item.model.IsLauncher === true || (url.length && tasks.launcherPosition(item.model.LauncherUrlWithoutIcon) >= 0)
        const items = []
        if (item.model.IsWindow) {
            items.push({ text: qsTr("New Window"), action: () => tasks.requestNewInstance(idx) })
            items.push({ separator: true })
        }
        items.push({ text: qsTr("Options"), children: [
            { text: qsTr("Keep in Dock"), checked: pinned, action: () => {
                if (pinned) tasks.requestRemoveLauncher(item.model.LauncherUrlWithoutIcon)
                else tasks.requestAddLauncher(item.model.LauncherUrlWithoutIcon)
            } }
        ] })
        if (item.model.IsWindow) {
            items.push({ separator: true })
            items.push({ text: qsTr("Hide"), action: () => tasks.requestToggleMinimized(idx) })
            items.push({ text: qsTr("Quit"), action: () => tasks.requestClose(idx) })
        } else {
            items.push({ text: qsTr("Open"), action: () => tasks.requestActivate(idx) })
        }
        const p = icon.mapToItem(null, icon.width / 2, 0)
        ShellState.openMenu("dock-" + item.index, p.x, items, { above: true, centered: true,
            y: (item.dock.targetScreen ? item.dock.targetScreen.height : 0) - item.dock.height + p.y })
    }
}
