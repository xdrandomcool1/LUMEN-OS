// SPDX-License-Identifier: GPL-3.0-or-later
// Lumen Finder main window: sidebar + tabs + browser views.
import QtQuick
import QtCore
import Lumen.Design
import Lumen.Motion
import Lumen.Glass
import Lumen.Controls
import Lumen.Finder

AppWindow {
    id: win
    property url startUrl
    title: current ? current.model.title : "Finder"
    width: 1040; height: 660
    visible: true
    contentColor: Theme.contentBackground

    property int currentTab: 0
    readonly property var current: tabsRepeater.count > currentTab ? tabsRepeater.itemAt(currentTab) : null
    ListModel { id: tabs }
    function newTab(url) { tabs.append({ startUrl: url.toString() }); currentTab = tabs.count - 1 }
    function closeTab(i) { if (tabs.count <= 1) { win.close(); return } tabs.remove(i); currentTab = Math.min(currentTab, tabs.count - 1) }
    function go(url) { if (current) current.navigate(url) }
    Component.onCompleted: newTab(startUrl)

    Settings { id: prefs; category: "Finder"; property int viewMode: 0; property bool showHidden: false }

    sidebar: FinderSidebar {
        currentUrl: win.current ? win.current.model.url : ""
        onOpen: url => win.go(url)
    }

    toolbarLeading: [
        ToolbarGroup {
            ToolbarButton { symbol: "caret-left"; enabled: win.current && win.current.canBack; tooltip: qsTr("Back"); onClicked: win.current.back() }
            ToolbarButton { symbol: "caret-right"; enabled: win.current && win.current.canForward; tooltip: qsTr("Forward"); onClicked: win.current.forward() }
        }
    ]
    toolbarTrailing: [
        ToolbarGroup {
            ToolbarButton { symbol: "squares-four"; checked: prefs.viewMode === 0; tooltip: qsTr("Icons"); onClicked: prefs.viewMode = 0 }
            ToolbarButton { symbol: "list"; checked: prefs.viewMode === 1; tooltip: qsTr("List"); onClicked: prefs.viewMode = 1 }
        },
        ToolbarGroup {
            ToolbarButton { symbol: "folder-simple"; tooltip: qsTr("New Folder"); onClicked: win.current.newFolder() }
            ToolbarButton { symbol: "trash"; visible: win.current && win.current.model.isTrash; tooltip: qsTr("Empty Trash"); onClicked: win.current.model.emptyTrash() }
            ToolbarButton { symbol: "dots-three-circle"; tooltip: qsTr("More"); onClicked: win.current.showBackgroundMenu() }
        },
        SearchField {
            width: 190; height: 32
            anchors.verticalCenter: parent ? parent.verticalCenter : undefined
            onTextChanged: if (win.current) win.current.model.filterText = text
        }
    ]

    // ---- Tab bar (shown when more than one tab) --------------------------
    Row {
        id: tabBar
        visible: tabs.count > 1
        x: 10; y: Theme.metrics.toolbarHeight
        width: parent.width - 20; height: visible ? 30 : 0
        spacing: 4
        Repeater {
            model: tabs
            delegate: Rectangle {
                required property int index
                width: (tabBar.width - (tabs.count - 1) * 4) / tabs.count; height: 28; radius: 8
                color: index === win.currentTab ? Theme.controlFillPressed : Theme.controlFill
                LText { anchors.centerIn: parent; text: tabsRepeater.itemAt(index) ? tabsRepeater.itemAt(index).model.title : ""; width: Math.min(implicitWidth, parent.width - 40) }
                Symbol { anchors.left: parent.left; anchors.leftMargin: 8; anchors.verticalCenter: parent.verticalCenter; width: 11; height: 11; name: "x"; weight: Symbol.Bold; color: Theme.secondaryLabel
                    MouseArea { anchors.fill: parent; anchors.margins: -4; onClicked: win.closeTab(index) } }
                MouseArea { anchors.fill: parent; z: -1; onClicked: win.currentTab = index }
            }
        }
    }

    Repeater {
        id: tabsRepeater
        model: tabs
        delegate: Browser {
            required property int index
            required property string startUrl
            initialUrl: startUrl
            visible: index === win.currentTab
            x: 0; y: Theme.metrics.toolbarHeight + tabBar.height
            width: win.contentArea.width; height: win.contentArea.height - y
            viewMode: prefs.viewMode
            showHidden: prefs.showHidden
            onOpenInNewTab: url => win.newTab(url)
            onToggleHidden: prefs.showHidden = !prefs.showHidden
        }
    }

    Shortcut { sequence: "Ctrl+T"; onActivated: win.newTab(win.current ? win.current.model.url : win.startUrl) }
    Shortcut { sequence: "Ctrl+W"; onActivated: win.closeTab(win.currentTab) }
    Shortcut { sequence: "Ctrl+1"; onActivated: prefs.viewMode = 0 }
    Shortcut { sequence: "Ctrl+2"; onActivated: prefs.viewMode = 1 }
    Shortcut { sequence: "Ctrl+Shift+."; onActivated: prefs.showHidden = !prefs.showHidden }
    Shortcut { sequence: "Ctrl+Shift+H"; onActivated: win.go("file://" + StandardPaths.writableLocation(StandardPaths.HomeLocation).toString().replace("file://", "")) }
    Shortcut { sequence: "Ctrl+Shift+D"; onActivated: win.go(StandardPaths.writableLocation(StandardPaths.DesktopLocation)) }
}
