// SPDX-License-Identifier: GPL-3.0-or-later
// Launchpad: full-screen grid of every installed application over a
// darkened, blurred wallpaper. GPU tiers refract/blur the wallpaper through
// PanelHost's shared BackdropBlur; the software tier falls back to the plain
// scrim so it stays responsive.
import QtQuick
import Lumen.Design
import Lumen.Motion
import Lumen.Glass
import Lumen.Controls
import Lumen.Shell
import Lumen.System

Item {
    id: lp
    property bool shown: false
    //! PanelHost's shared wallpaper blur (null on the software tier).
    property BackdropBlur backdrop: null
    property bool compositorBlur: false

    readonly property var apps: AppsModel.search(field.text, 400)
    onAppsChanged: if (grid.currentIndex >= apps.length) grid.currentIndex = -1

    onShownChanged: {
        if (shown) {
            field.text = ""
            grid.currentIndex = -1
            field.forceActiveFocus()
        }
    }

    Spring { id: appear; to: lp.shown ? 1 : 0; preset: Spring.Snappy }
    opacity: appear.value
    visible: appear.value > 0.005

    // Dimmed wallpaper (the blurred backdrop comes from PanelHost).
    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.45)
    }

    // ---- Search -----------------------------------------------------------
    Item {
        id: searchWrap
        anchors.horizontalCenter: parent.horizontalCenter
        y: 34
        width: 300; height: 36
        GlassSurface {
            anchors.fill: parent
            radius: height / 2
            material: Glass.Floating
            backdrop: lp.backdrop
            compositorBlur: lp.compositorBlur
            tintAlpha: lp.backdrop || lp.compositorBlur ? -1 : 0.5
        }
        Symbol {
            x: 12; anchors.verticalCenter: parent.verticalCenter
            width: 14; height: 14; name: "magnifying-glass"; color: Theme.onWallpaper
        }
        TextField {
            id: field
            anchors.fill: parent
            leftPadding: 34
            background: null
            placeholderText: qsTr("Search")
            color: Theme.onWallpaper
            placeholderTextColor: Qt.rgba(1, 1, 1, 0.55)
            onAccepted: {
                if (grid.currentIndex >= 0) grid.launchCurrent()
                else if (lp.apps.length) lp.launchApp(lp.apps[0])
            }
            Keys.onDownPressed: { grid.currentIndex = 0; grid.forceActiveFocus() }
        }
    }

    // ---- App grid ---------------------------------------------------------
    GridView {
        id: grid
        anchors.top: searchWrap.bottom
        anchors.topMargin: 26
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(lp.width - 140, 7 * cellWidth)
        height: parent.height - anchors.topMargin - searchWrap.height - 30
        cellWidth: 148; cellHeight: 152
        clip: true
        model: lp.apps
        boundsBehavior: Flickable.StopAtBounds
        keyNavigationEnabled: true

        function launchCurrent() {
            if (currentIndex >= 0 && currentIndex < lp.apps.length)
                lp.launchApp(lp.apps[currentIndex])
        }

        delegate: Item {
            id: tile
            width: grid.cellWidth; height: grid.cellHeight
            required property var modelData
            required property int index
            readonly property bool hot: ma.containsMouse || grid.currentIndex === tile.index

            // Hover plate behind the tile (Launchpad selection look).
            Rectangle {
                z: -1
                x: 4; y: 4; width: tile.width - 8; height: tile.height - 14
                radius: Theme.metrics.hoverRadius
                color: tile.hot ? Qt.rgba(1, 1, 1, 0.16) : "transparent"
                opacity: 0.9
            }
            Column {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top; anchors.topMargin: 10
                spacing: 9
                AppIcon {
                    id: icon
                    anchors.horizontalCenter: parent.horizontalCenter
                    size: 92
                    source: tile.modelData.iconName
                    Spring { id: z; to: tile.hot ? 1 : 0; preset: Spring.Snappy }
                    scale: 1 + 0.08 * z.value
                }
                LText {
                    width: 140
                    horizontalAlignment: Text.AlignHCenter
                    text: tile.modelData.name
                    color: Theme.onWallpaper
                    style: LText.Caption
                    elide: Text.ElideMiddle
                }
            }
            MouseArea {
                id: ma
                anchors.fill: parent
                hoverEnabled: true
                onClicked: lp.launchApp(tile.modelData)
            }
        }
    }

    function launchApp(app) {
        AppsModel.launch(app.desktopId)
        ShellController.ClosePanels()
    }
}
