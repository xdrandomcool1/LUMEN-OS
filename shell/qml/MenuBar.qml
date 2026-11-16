// SPDX-License-Identifier: GPL-3.0-or-later
// Top menu bar. Transparent over the wallpaper (optional glass strip),
// text colour adapts to the wallpaper's luminance under the bar.
import QtQuick
import QtQuick.Window
import Lumen.Design
import Lumen.Motion
import Lumen.Glass
import Lumen.Shell
import Lumen.System

Window {
    id: win
    property var targetScreen
    screen: targetScreen
    visible: false
    color: "transparent"
    title: "lumen-menubar"
    width: targetScreen ? targetScreen.width : 1280
    height: Theme.metrics.menuBarHeight

    readonly property bool darkText: !ShellSettings.menuBarBackground && WallpaperEngine.topLuminance > 0.62 && !Theme.dark
                                     || (ShellSettings.menuBarBackground && !Theme.dark)
    readonly property color fg: darkText ? Qt.rgba(0, 0, 0, 0.85) : Qt.rgba(1, 1, 1, 0.96)
    readonly property string openMenu: ShellController.activePanel.startsWith("menu:") ? ShellController.activePanel.substring(5) : ""

    LayerSurface {
        window: win
        screen: win.targetScreen
        layer: LayerSurface.Top
        anchors: LayerSurface.AnchorTop | LayerSurface.AnchorLeft | LayerSurface.AnchorRight
        exclusiveZone: Theme.metrics.menuBarHeight
        scope: "lumen-menubar"
    }

    // Optional glass strip; also a faint top gradient so white text stays
    // legible on busy wallpapers (as with the reference design).
    Rectangle {
        anchors.fill: parent
        visible: !ShellSettings.menuBarBackground && !win.darkText
        gradient: Gradient {
            GradientStop { position: 0; color: Qt.rgba(0, 0, 0, 0.16) }
            GradientStop { position: 1; color: Qt.rgba(0, 0, 0, 0.0) }
        }
    }
    Rectangle {
        anchors.fill: parent
        visible: ShellSettings.menuBarBackground
        color: Theme.dark ? Qt.rgba(0.12, 0.12, 0.13, 0.72) : Qt.rgba(0.97, 0.97, 0.98, 0.72)
    }

    component BarItem: Item {
        id: bi
        property string menuId: ""
        property bool bold: false
        property alias text: label.text
        property string symbol: ""
        property real symbolSize: 16
        property int symbolWeight: Symbol.Regular
        signal clicked(real x)
        readonly property bool open: menuId.length > 0 && win.openMenu === menuId
        height: win.height
        implicitWidth: row.implicitWidth + 20
        Rectangle {
            anchors.fill: parent; anchors.topMargin: 3; anchors.bottomMargin: 3
            radius: 6
            color: win.darkText ? Qt.rgba(0, 0, 0, 0.12) : Qt.rgba(1, 1, 1, 0.22)
            opacity: bi.open || ma.pressed ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: Motion.instant } }
        }
        Row {
            id: row
            anchors.centerIn: parent
            spacing: 5
            Symbol {
                visible: bi.symbol.length > 0
                name: bi.symbol; weight: bi.symbolWeight
                width: bi.symbolSize; height: bi.symbolSize
                color: win.fg
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                id: label
                visible: text.length > 0
                color: win.fg
                font.family: Theme.fontFamily
                font.pixelSize: Theme.type.menuBar
                font.weight: bi.bold ? Font.Bold : Font.Medium
                font.letterSpacing: -0.08
                anchors.verticalCenter: parent.verticalCenter
                renderType: Text.QtRendering
                style: win.darkText ? Text.Normal : Text.Raised
                styleColor: Qt.rgba(0, 0, 0, 0.12)
            }
        }
        MouseArea {
            id: ma
            anchors.fill: parent
            hoverEnabled: true
            onClicked: bi.clicked(bi.mapToItem(null, 0, 0).x)
            // Sliding across the bar while a menu is open switches menus.
            onContainsMouseChanged: if (containsMouse && win.openMenu.length && bi.menuId.length && !bi.open) bi.clicked(bi.mapToItem(null, 0, 0).x)
        }
    }

    // ---- Leading: Lumen menu, app name, application menus ------------------
    Row {
        id: leading
        x: 8
        height: parent.height
        BarItem {
            menuId: "lumen"
            symbol: "lumen-logo"; symbolWeight: Symbol.Fill; symbolSize: 15
            implicitWidth: 34
            onClicked: x => ShellState.openMenu("lumen", x, MenuDefinitions.lumenMenu())
        }
        BarItem {
            menuId: "app"
            text: ShellState.activeAppName
            bold: true
            onClicked: x => ShellState.openMenu("app", x, MenuDefinitions.appMenu(ShellState.activeAppName))
        }
        Repeater {
            model: AppMenu.menus
            delegate: BarItem {
                required property var modelData
                required property int index
                menuId: "appmenu-" + index
                text: modelData.title
                onClicked: x => ShellState.openMenu("appmenu-" + index, x, AppMenu.items(index))
            }
        }
    }

    // ---- Trailing: status items + clock ------------------------------------
    Row {
        anchors.right: parent.right
        anchors.rightMargin: 8
        height: parent.height

        BarItem {
            visible: KeyboardLayout.layouts.length > 1
            text: KeyboardLayout.shortName
            onClicked: KeyboardLayout.next()
        }
        BarItem {
            visible: Battery.present
            menuId: "battery"
            symbol: Battery.symbol; symbolSize: 20
            text: ShellSettings.showBatteryPercentage ? Math.round(Battery.percentage) + "%" : ""
            onClicked: x => ShellState.openMenu("battery", x, MenuDefinitions.batteryMenu())
        }
        BarItem {
            menuId: "wifi"
            symbol: Network.symbol; symbolWeight: Symbol.Bold; symbolSize: 16
            onClicked: ShellController.ShowPanel("controlcenter:wifi")
        }
        BarItem {
            symbol: "magnifying-glass"; symbolWeight: Symbol.Bold; symbolSize: 15
            onClicked: ShellController.ToggleSpotlight()
        }
        BarItem {
            menuId: "cc"
            symbol: "toggle-left"; symbolWeight: Symbol.Fill; symbolSize: 17
            onClicked: ShellController.ToggleControlCenter()
            Component.onCompleted: symbol = "sliders-horizontal"
        }
        BarItem {
            text: Qt.formatDateTime(ShellState.now, ShellSettings.clock24h ? "ddd d MMM  HH:mm" : "ddd d MMM  h:mm AP")
            onClicked: ShellController.ToggleNotificationCenter()
        }
    }

    Component.onCompleted: visible = true
}
