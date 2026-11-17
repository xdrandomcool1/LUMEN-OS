// SPDX-License-Identifier: GPL-3.0-or-later
// Overlay surface hosting transient panels: menu-bar menus, Control Center,
// Spotlight and Notification Center. It is only mapped while a panel is
// open, takes keyboard focus, and dismisses on outside click or Escape.
import QtQuick
import QtQuick.Window
import Lumen.Design
import Lumen.Motion
import Lumen.Glass
import Lumen.Controls
import Lumen.Shell
import Lumen.System

Window {
    id: win
    property var targetScreen
    property bool primary: true
    screen: targetScreen
    visible: false
    color: "transparent"
    title: "lumen-panels"
    width: targetScreen ? targetScreen.width : 1280
    height: targetScreen ? targetScreen.height : 800

    readonly property string panel: primary ? ShellController.activePanel : ""
    readonly property bool menuOpen: panel.startsWith("menu:")
    readonly property bool open: panel.length > 0

    //! Rect (window coordinates) of the panel currently shown.
    property rect activeRect: Qt.rect(0, 0, 0, 0)
    property real activeRadius: 20
    //! Material of the panel currently shown (for compositor glass).
    property int activeMaterial: Glass.Regular

    LayerSurface {
        id: layer
        window: win
        screen: win.targetScreen
        layer: LayerSurface.Overlay
        anchors: LayerSurface.AnchorTop | LayerSurface.AnchorBottom | LayerSurface.AnchorLeft | LayerSurface.AnchorRight
        exclusiveZone: -1
        keyboard: LayerSurface.KeyboardExclusive
        scope: "lumen-panels"
    }

    // In-process backdrop: the wallpaper (never shown, only refracted).
    Wallpaper { id: wall; anchors.fill: parent; animate: false }
    BackdropBlur { id: blur; source: wall; hideSource: true }

    // Use compositor blur whenever a window is underneath the open panel.
    RegionWatcher {
        id: occlusion
        model: ShellState.windows
        region: win.activeRect
    }
    readonly property BackdropBlur glassBackdrop: occlusion.occupied || kwinGlass.active ? null : blur
    readonly property bool glassCompositor: occlusion.occupied || kwinGlass.active

    // Compositor-side Liquid Glass behind the open panel (see Dock.qml).
    readonly property bool lumenCompositorGlass: kwinGlass.active
    readonly property var activeParams: Glass.material(win.activeMaterial, Appearance.dark, Appearance.accent)
    CompositorGlass {
        id: kwinGlass
        scope: "lumen-panels"
        output: win.targetScreen ? win.targetScreen.name : ""
        enabled: win.open && win.activeRect.width > 0
        shapes: [{
            x: win.activeRect.x, y: win.activeRect.y, w: win.activeRect.width, h: win.activeRect.height,
            r: win.activeRadius, n: 2.6,
            thick: win.activeParams.refraction * Appearance.glassIntensity, lens: win.activeParams.lensWidth,
            disp: win.activeParams.dispersion, ior: win.activeParams.ior, frost: win.activeParams.frost,
            sat: win.activeParams.saturation, bri: win.activeParams.brightness
        }]
    }

    WindowEffects {
        window: win
        shadow: false
        blurBehind: !kwinGlass.active && win.open && occlusion.occupied && win.activeRect.width > 0
        blurRect: win.activeRect
        radius: win.activeRadius
    }

    // Hide the surface after the exit animation finished.
    onOpenChanged: {
        if (open) { hideTimer.stop(); win.visible = true; focusScope.forceActiveFocus() }
        else hideTimer.restart()
    }
    Timer { id: hideTimer; interval: Motion.normal + 40; onTriggered: win.visible = false }

    FocusScope {
        id: focusScope
        anchors.fill: parent
        focus: true
        Keys.onEscapePressed: ShellController.ClosePanels()

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
            onPressed: ShellController.ClosePanels()
        }

        // ---- Menu-bar dropdowns ---------------------------------------
        Item {
            id: menuSlot
            readonly property bool shown: win.menuOpen && ShellState.menuRequest !== null
            visible: menuFade.value > 0.01
            opacity: menuFade.value
            Spring { id: menuFade; to: menuSlot.shown ? 1 : 0; preset: Spring.Snappy }
            MenuPanel {
                id: menu
                readonly property var req: ShellState.menuRequest
                x: Math.max(8, Math.min(req ? (req.centered ? req.x - width / 2 : req.x) : 0, win.width - width - 8))
                y: req && req.above ? req.y - height - 10 : Theme.metrics.menuBarHeight + 3
                items: ShellState.menuRequest ? ShellState.menuRequest.items : []
                backdrop: win.glassBackdrop
                compositorBlur: win.glassCompositor
                onDone: ShellController.ClosePanels()
                onItemsChanged: currentIndex = -1
                MouseArea { anchors.fill: parent; z: -1; onPressed: mouse => mouse.accepted = true }
            }
            onShownChanged: if (shown) { win.activeRect = Qt.rect(menu.x, menu.y, menu.width, menu.height); win.activeRadius = menu.radius; win.activeMaterial = Glass.Thin }
            Keys.forwardTo: []
        }

        Keys.onUpPressed: if (win.menuOpen) menu.move(-1)
        Keys.onDownPressed: if (win.menuOpen) menu.move(1)
        Keys.onReturnPressed: if (win.menuOpen) menu.activate(menu.currentIndex)

        // ---- Control Center -------------------------------------------
        ControlCenter {
            id: cc
            shown: win.panel.startsWith("controlcenter")
            section: win.panel.indexOf(":") > 0 ? win.panel.split(":")[1] : ""
            anchors.right: parent.right; anchors.rightMargin: 10
            y: Theme.metrics.menuBarHeight + 6
            backdrop: win.glassBackdrop
            compositorBlur: win.glassCompositor
            onShownChanged: if (shown) { win.activeRect = Qt.rect(x, y, width, height); win.activeRadius = Theme.metrics.panelRadius; win.activeMaterial = Glass.Regular }
            onHeightChanged: if (shown) win.activeRect = Qt.rect(x, y, width, height)
        }

        // ---- Spotlight ------------------------------------------------
        Spotlight {
            id: spotlight
            shown: win.panel.startsWith("spotlight") || win.panel === "launchpad"
            initialQuery: win.panel.startsWith("spotlight:") ? win.panel.substring(10) : ""
            anchors.horizontalCenter: parent.horizontalCenter
            y: Math.round(win.height * 0.22)
            backdrop: win.glassBackdrop
            compositorBlur: win.glassCompositor
            onShownChanged: if (shown) { win.activeRect = Qt.rect(x, y, width, 400); win.activeRadius = 27; win.activeMaterial = Glass.Frosted }
        }

        // ---- Notification Center --------------------------------------
        NotificationCenter {
            id: nc
            shown: win.panel === "notifications"
            anchors.right: parent.right; anchors.rightMargin: 10
            y: Theme.metrics.menuBarHeight + 6
            height: win.height - y - 10
            backdrop: win.glassBackdrop
            compositorBlur: win.glassCompositor
            onShownChanged: if (shown) { win.activeRect = Qt.rect(x, y, width, height); win.activeRadius = 22; win.activeMaterial = Glass.Regular }
        }
    }
}
