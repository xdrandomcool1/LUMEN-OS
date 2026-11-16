// SPDX-License-Identifier: GPL-3.0-or-later
// Background layer: wallpaper (+ desktop widgets).
import QtQuick
import QtQuick.Window
import Lumen.Glass
import Lumen.Shell

Window {
    id: win
    property var targetScreen
    screen: targetScreen
    visible: false
    color: "black"
    title: "lumen-desktop"
    width: targetScreen ? targetScreen.width : 1280
    height: targetScreen ? targetScreen.height : 800

    LayerSurface {
        window: win
        screen: win.targetScreen
        layer: LayerSurface.Background
        anchors: LayerSurface.AnchorTop | LayerSurface.AnchorBottom | LayerSurface.AnchorLeft | LayerSurface.AnchorRight
        exclusiveZone: -1
        scope: "lumen-desktop"
    }
    Wallpaper { id: wall; anchors.fill: parent }
    BackdropBlur { id: blur; source: wall }
    DesktopWidgets { anchors.fill: parent; anchors.topMargin: 40; backdrop: blur }
    Component.onCompleted: visible = true
}
