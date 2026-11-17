// SPDX-License-Identifier: GPL-3.0-or-later
// Lumen Settings.
import QtQuick
import Lumen.Design
import Lumen.Motion
import Lumen.Glass
import Lumen.Controls
import Lumen.Settings

AppWindow {
    id: win
    property string startPage: "appearance"
    title: currentTitle
    width: 900; height: 680
    minimumWidth: 760
    visible: true
    contentColor: Theme.windowBackground

    readonly property var pages: [
        { id: "network", title: qsTr("Network"), symbol: "wifi-high", tint: Theme.blue, keys: "wifi wi-fi ethernet ip internet" },
        { id: "bluetooth", title: qsTr("Bluetooth"), symbol: "bluetooth", tint: Theme.blue, keys: "devices headphones pairing" },
        { sep: true },
        { id: "appearance", title: qsTr("Appearance"), symbol: "circle-half", tint: Theme.gray, keys: "dark light accent color glass" },
        { id: "wallpaper", title: qsTr("Wallpaper"), symbol: "image", tint: Theme.cyan, keys: "background picture dynamic" },
        { id: "dock", title: qsTr("Desktop & Dock"), symbol: "app-window", tint: Theme.indigo, keys: "dock magnification menu bar widgets weather" },
        { id: "displays", title: qsTr("Displays"), symbol: "monitor", tint: Theme.blue, keys: "resolution scale refresh night shift" },
        { id: "accessibility", title: qsTr("Accessibility"), symbol: "person-arms-spread", tint: Theme.blue, keys: "motion transparency contrast" },
        { sep: true },
        { id: "sound", title: qsTr("Sound"), symbol: "speaker-high", tint: Theme.pink, keys: "volume output audio" },
        { id: "keyboard", title: qsTr("Keyboard"), symbol: "keyboard", tint: Theme.gray, keys: "layout input source shortcuts repeat" },
        { id: "mouse", title: qsTr("Mouse & Trackpad"), symbol: "mouse-simple", tint: Theme.gray, keys: "pointer touchpad scroll tap natural" },
        { id: "power", title: qsTr("Battery"), symbol: "battery-high", tint: Theme.green, keys: "power energy sleep low power mode" },
        { sep: true },
        { id: "users", title: qsTr("Users & Groups"), symbol: "users", tint: Theme.gray, keys: "account password avatar name" },
        { id: "privacy", title: qsTr("Privacy & Security"), symbol: "shield-check", tint: Theme.blue, keys: "firewall lock screen password" },
        { id: "about", title: qsTr("About"), symbol: "info", tint: Theme.gray, keys: "system computer name hostname version hardware" }
    ]
    property string current: startPage
    readonly property string currentTitle: (pages.find(p => p.id === current) || {}).title || "Settings"

    sidebar: Column {
        spacing: 2
        SearchField { id: search; x: 10; width: parent.width - 20; placeholderText: qsTr("Search") }
        Item { width: 1; height: 6 }
        Repeater {
            model: win.pages
            delegate: Loader {
                required property var modelData
                width: parent.width
                readonly property bool matches: !search.text.length || (modelData.title + " " + (modelData.keys || "")).toLowerCase().includes(search.text.toLowerCase())
                active: modelData.sep ? !search.text.length : matches
                visible: active
                sourceComponent: modelData.sep ? sepC : itemC
                Component { id: sepC; Item { height: 10 } }
                Component {
                    id: itemC
                    Item {
                        height: 30
                        Rectangle { anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; radius: 8
                                    color: win.current === modelData.id ? Theme.controlFillPressed : (ma.containsMouse ? Theme.controlFill : "transparent") }
                        Rectangle { x: 16; anchors.verticalCenter: parent.verticalCenter; width: 22; height: 22; radius: 6; color: modelData.tint
                            Symbol { anchors.centerIn: parent; width: 14; height: 14; name: modelData.symbol; weight: Symbol.Fill; color: "white" } }
                        LText { x: 46; anchors.verticalCenter: parent.verticalCenter; text: modelData.title }
                        MouseArea { id: ma; anchors.fill: parent; hoverEnabled: true; onClicked: win.current = modelData.id }
                    }
                }
            }
        }
    }

    Loader {
        id: page
        x: 0; y: Theme.metrics.toolbarHeight
        width: win.contentArea.width; height: win.contentArea.height - y
        source: "Page" + win.current.charAt(0).toUpperCase() + win.current.slice(1) + ".qml"
        opacity: 0
        onLoaded: fadeIn.restart()
        NumberAnimation { id: fadeIn; target: page; property: "opacity"; from: 0; to: 1; duration: Motion.fast }
    }
}
