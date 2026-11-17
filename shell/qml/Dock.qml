// SPDX-License-Identifier: GPL-3.0-or-later
// The Dock: floating Liquid Glass shelf of pinned and running apps.
import QtQuick
import QtQuick.Window
import QtCore
import Qt.labs.folderlistmodel
import Lumen.Design
import Lumen.Motion
import Lumen.Glass
import Lumen.Controls
import Lumen.Shell
import Lumen.System
import Lumen.Shell.Plasma

Window {
    id: win
    property var targetScreen
    screen: targetScreen
    visible: false
    color: "transparent"
    title: "lumen-dock"

    // ---- Geometry tokens ----------------------------------------------------
    readonly property real base: ShellSettings.dockIconSize
    readonly property real mag: ShellSettings.dockMagnification ? Math.max(base, ShellSettings.dockMagnifiedSize) : base
    readonly property real spacing: Math.round(base * 0.12)
    readonly property real pad: Math.round(base * 0.13)
    readonly property real gap: Theme.metrics.dockGap
    readonly property real tile: base + spacing
    readonly property real sepW: 13
    readonly property real glassH: base + 2 * pad
    readonly property real labelSpace: 46
    readonly property int appCount: apps.count

    width: targetScreen ? targetScreen.width : 1280
    height: Math.ceil(mag + 2 * pad + gap + labelSpace)

    // ---- Hover / magnification ---------------------------------------------
    property real pointerX: -1
    Spring { id: hover; to: dockHover.hovered && !dragging ? 1 : 0; preset: Spring.Critical; stiffness: 600; damping: 49 }
    property bool dragging: false

    readonly property real baseWidth: appCount * tile + sepW + tile + 2 * pad
    readonly property real baseLeft: (width - baseWidth) / 2
    //! Pointer in unmagnified layout coordinates.
    readonly property real pointerBase: pointerX - baseLeft
    readonly property real range: tile * 3.1

    function baseCenter(i) {
        // Items 0..appCount-1 are apps; appCount is Trash (after separator).
        return pad + (i < appCount ? i * tile : appCount * tile + sepW) + tile / 2
    }
    function factor(i) {
        if (hover.value <= 0.001 || pointerX < 0) return 0
        const d = Math.abs(pointerBase - baseCenter(i))
        if (d >= range) return 0
        return (1 + Math.cos(Math.PI * d / range)) / 2
    }
    function sizeFor(i) { return base + (mag - base) * factor(i) * hover.value }

    readonly property real totalWidth: {
        let w = 2 * pad + sepW
        for (let i = 0; i <= appCount; ++i) w += sizeFor(i) + spacing
        return w
    }

    // ---- Auto-hide ---------------------------------------------------------
    property bool revealed: !ShellSettings.dockAutoHide || dockHover.hovered || edgeHover.hovered || dragging || ShellController.activePanel.startsWith("menu:dock")
    Timer { id: hideDelay; interval: 650; onTriggered: win.hiddenTarget = 1 }
    property real hiddenTarget: 0
    onRevealedChanged: { if (revealed) { hideDelay.stop(); hiddenTarget = 0 } else hideDelay.restart() }
    Spring { id: hidden; to: ShellSettings.dockAutoHide ? win.hiddenTarget : 0; preset: Spring.Smooth }
    readonly property real slide: hidden.value * (glassH + gap + 12)

    readonly property real glassTop: height - gap - glassH + slide
    readonly property rect dockRect: Qt.rect((width - totalWidth) / 2, glassTop, totalWidth, glassH)

    LayerSurface {
        window: win
        screen: win.targetScreen
        layer: LayerSurface.Top
        anchors: LayerSurface.AnchorBottom | LayerSurface.AnchorLeft | LayerSurface.AnchorRight
        exclusiveZone: ShellSettings.dockAutoHide ? 0 : Math.round(win.glassH + win.gap)
        scope: "lumen-dock"
        inputRegion: hidden.value > 0.95
                     ? Qt.rect(0, win.height - 3, win.width, 3)
                     : Qt.rect(win.dockRect.x - 4, win.dockRect.y - (win.mag - win.base) * hover.value - 4,
                               win.dockRect.width + 8, win.height - win.dockRect.y + (win.mag - win.base) * hover.value + 4)
    }

    // ---- Backdrop: wallpaper slice behind the Dock --------------------------
    Wallpaper {
        id: wall
        animate: false
        width: win.targetScreen ? win.targetScreen.width : win.width
        height: win.targetScreen ? win.targetScreen.height : 800
        y: win.height - height
    }
    BackdropBlur { id: blur; source: wall; hideSource: true; strength: 0.8 }

    RegionWatcher {
        id: occlusion
        model: ShellState.windows
        region: Qt.rect((win.targetScreen ? win.targetScreen.virtualX : 0) + win.dockRect.x,
                        (win.targetScreen ? win.targetScreen.virtualY + win.targetScreen.height - win.height : 0) + win.dockRect.y,
                        win.dockRect.width, win.dockRect.height)
    }
    // Compositor-side Liquid Glass (KWin effect lumen_glass): refracts the
    // real content behind the Dock, windows included. Without it, fall back
    // to refracting the wallpaper in-process and to compositor blur when a
    // window is underneath.
    readonly property bool lumenCompositorGlass: kwinGlass.active
    CompositorGlass {
        id: kwinGlass
        scope: "lumen-dock"
        output: win.targetScreen ? win.targetScreen.name : ""
        enabled: hidden.value < 0.95
        shapes: [ dockGlass.compositorShape(Qt.rect(win.dockRect.x, win.dockRect.y, win.dockRect.width, win.glassH)) ]
    }
    WindowEffects {
        window: win
        shadow: false
        blurBehind: !kwinGlass.active && occlusion.occupied && hidden.value < 0.95
        blurRect: win.dockRect
        radius: dockGlass.radius
    }

    HoverHandler { id: edgeHover; enabled: ShellSettings.dockAutoHide }

    // ---- The shelf ---------------------------------------------------------
    GlassSurface {
        id: dockGlass
        x: win.dockRect.x
        y: win.dockRect.y
        width: win.dockRect.width
        height: win.glassH
        radius: Math.round(height * 0.42)
        material: Glass.UltraThin
        backdrop: occlusion.occupied || kwinGlass.active ? null : blur
        compositorBlur: occlusion.occupied || kwinGlass.active
        shadowScale: 1.0
        shadowOffsetY: 8
    }

    Item {
        id: dockArea
        x: win.dockRect.x
        width: win.dockRect.width
        y: win.dockRect.y - (win.mag - win.base) - 8
        height: win.height - y
        HoverHandler {
            id: dockHover
            onPointChanged: win.pointerX = point.position.x + dockArea.x
        }
    }

    Row {
        id: row
        x: win.dockRect.x + win.pad
        y: win.dockRect.y + win.pad + win.base   // baseline (bottom of icons)
        Repeater {
            id: apps
            model: ShellState.tasks
            delegate: DockItem {
                dock: win
                tasks: ShellState.tasks
            }
        }
        Item { // separator
            width: win.sepW
            height: 1
            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                y: -win.base * 0.86
                width: 1; height: win.base * 0.72
                color: Theme.dark ? Qt.rgba(1, 1, 1, 0.22) : Qt.rgba(0, 0, 0, 0.16)
            }
        }
        DockTrash { dock: win }
    }

    Component.onCompleted: visible = true
}
