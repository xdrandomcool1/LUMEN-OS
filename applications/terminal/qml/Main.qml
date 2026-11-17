// SPDX-License-Identifier: GPL-3.0-or-later
// Lumen Terminal.
import QtQuick
import QtCore
import Lumen.Design
import Lumen.Motion
import Lumen.Controls
import Lumen.Terminal

AppWindow {
    id: win
    property string startDir: ""
    property var startCommand: []
    title: current ? current.title : "Terminal"
    subtitle: current ? current.columns + " × " + current.rows : ""
    width: 820; height: 520
    visible: true
    toolbarHeight: 46
    contentColor: current ? current.background : "#1A1B1F"

    Settings { id: prefs; category: "Terminal"; property string theme: Appearance.dark ? "Lumen Dark" : "Lumen Light"; property real fontSize: 13 }

    ListModel { id: tabs }
    property int currentTab: 0
    readonly property var current: stack.count > currentTab ? stack.itemAt(currentTab) : null
    function newTab(dir) { tabs.append({ dir: dir || (current ? current.currentDirectory() : startDir) }); currentTab = tabs.count - 1 }
    function closeTab(i) { if (tabs.count <= 1) { win.close(); return } tabs.remove(i); currentTab = Math.min(currentTab, tabs.count - 1) }
    Component.onCompleted: newTab(startDir)

    toolbarTrailing: [
        ToolbarGroup {
            ToolbarButton { symbol: "plus"; tooltip: qsTr("New Tab"); onClicked: win.newTab() }
            ToolbarButton { symbol: "palette"; tooltip: qsTr("Theme"); onClicked: themeMenu.popup() }
        }
    ]
    Menu {
        id: themeMenu
        Instantiator {
            model: ["Lumen Dark", "Lumen Light", "Midnight", "Solar"]
            delegate: MenuItem { required property string modelData; text: modelData; checkable: true; checked: prefs.theme === modelData; onTriggered: prefs.theme = modelData }
            onObjectAdded: (i, o) => themeMenu.insertItem(i, o)
            onObjectRemoved: (i, o) => themeMenu.removeItem(o)
        }
    }

    Row {
        id: tabBar
        visible: tabs.count > 1
        x: 8; y: win.toolbarHeight; width: parent.width - 16; height: visible ? 28 : 0
        spacing: 4
        Repeater {
            model: tabs
            delegate: Rectangle {
                required property int index
                width: (tabBar.width - (tabs.count - 1) * 4) / tabs.count; height: 26; radius: Theme.metrics.hoverRadius
                color: index === win.currentTab ? Qt.rgba(1, 1, 1, Theme.dark ? 0.12 : 0.6) : Qt.rgba(1, 1, 1, Theme.dark ? 0.04 : 0.25)
                LText { anchors.centerIn: parent; width: Math.min(implicitWidth, parent.width - 36); text: stack.itemAt(index) ? stack.itemAt(index).title : ""; style: LText.Callout }
                Symbol { x: 8; anchors.verticalCenter: parent.verticalCenter; width: 10; height: 10; name: "x"; weight: Symbol.Bold; color: Theme.secondaryLabel
                         MouseArea { anchors.fill: parent; anchors.margins: -5; onClicked: win.closeTab(index) } }
                MouseArea { anchors.fill: parent; z: -1; onClicked: win.currentTab = index }
            }
        }
    }

    Repeater {
        id: stack
        model: tabs
        delegate: TerminalItem {
            required property int index
            required property string dir
            x: 0; y: win.toolbarHeight + tabBar.height
            width: win.contentArea.width; height: win.contentArea.height - y
            visible: index === win.currentTab
            focus: visible
            workingDirectory: dir
            command: index === 0 ? win.startCommand : []
            theme: prefs.theme
            fontSize: prefs.fontSize
            Component.onCompleted: { start(); if (visible) forceActiveFocus() }
            onVisibleChanged: if (visible) forceActiveFocus()
            onFinished: win.closeTab(index)
        }
    }

    Shortcut { sequences: ["Ctrl+Shift+T", "Meta+T"]; onActivated: win.newTab() }
    Shortcut { sequences: ["Ctrl+Shift+W", "Meta+W"]; onActivated: win.closeTab(win.currentTab) }
    Shortcut { sequence: "Ctrl+PgDown"; onActivated: win.currentTab = (win.currentTab + 1) % tabs.count }
    Shortcut { sequence: "Ctrl+PgUp"; onActivated: win.currentTab = (win.currentTab + tabs.count - 1) % tabs.count }
    Shortcut { sequences: ["Ctrl+=", "Ctrl++"]; onActivated: prefs.fontSize = Math.min(32, prefs.fontSize + 1) }
    Shortcut { sequence: "Ctrl+-"; onActivated: prefs.fontSize = Math.max(8, prefs.fontSize - 1) }
    Shortcut { sequence: "Ctrl+0"; onActivated: prefs.fontSize = 13 }
    Shortcut { sequence: "Ctrl+Shift+K"; onActivated: if (win.current) win.current.clearScrollback() }
}
