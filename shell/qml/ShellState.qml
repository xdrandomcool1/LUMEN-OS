// SPDX-License-Identifier: GPL-3.0-or-later
// Shared shell state: task models, clock, active application.
pragma Singleton
import QtQuick
import Lumen.Shell
import Lumen.Shell.Plasma

QtObject {
    id: state

    //! Dock model: pinned launchers + running apps grouped per application.
    readonly property TasksModel tasks: TasksModel {
        groupMode: TasksModel.GroupApplications
        groupInline: false
        sortMode: TasksModel.SortManual
        launchInPlace: true
        separateLaunchers: false
        hideActivatedLaunchers: false
        filterByVirtualDesktop: false
        filterByScreen: false
        filterByActivity: false
        taskReorderingEnabled: true
        launcherList: ShellSettings.pinnedApps
        onLauncherListChanged: {
            if (JSON.stringify(launcherList) !== JSON.stringify(ShellSettings.pinnedApps))
                ShellSettings.pinnedApps = launcherList
        }
    }

    //! Flat list of windows on the current desktop (occlusion checks).
    readonly property TasksModel windows: TasksModel {
        groupMode: TasksModel.GroupDisabled
        filterByVirtualDesktop: true
        filterByCurrentVirtualDesktop: true
        filterMinimized: true
        filterByActivity: false
        sortMode: TasksModel.SortLastActivated
    }

    readonly property var activeTask: tasks.activeTask
    readonly property string activeAppName: {
        const idx = tasks.activeTask
        if (!idx || !idx.valid) return "Finder"
        const n = tasks.data(idx, AbstractTasksModel.AppName)
        return n && n.length ? n : "Finder"
    }
    readonly property string activeAppId: {
        const idx = tasks.activeTask
        return idx && idx.valid ? (tasks.data(idx, AbstractTasksModel.AppId) || "") : ""
    }

    property date now: new Date()
    readonly property Timer clockTimer: Timer {
        interval: 1000; running: true; repeat: true
        onTriggered: state.now = new Date()
    }

    //! Geometry of Dock icons per launcher/app id (for minimize animations
    //! and the "genie" target in KWin effects).
    property var dockIconRects: ({})

    //! Menu-bar dropdown requests: {id, x, items}
    property var menuRequest: null
    // Menus requested by name over D-Bus (keyboard access, automation).
    readonly property Connections panelWatch: Connections {
        target: ShellController
        function onActivePanelChanged() {
            const p = ShellController.activePanel
            if (!p.startsWith("menu:")) return
            const id = p.substring(5)
            if (state.menuRequest && state.menuRequest.id === id) return
            if (id === "lumen") state.openMenu("lumen", 8, MenuDefinitions.lumenMenu())
            else if (id === "app") state.openMenu("app", 42, MenuDefinitions.appMenu(state.activeAppName))
        }
    }
    //! opts: { y, above } — Dock menus open above their icon.
    function openMenu(id, x, items, opts) {
        const o = opts || {}
        menuRequest = { id: id, x: x, items: items, y: o.y !== undefined ? o.y : -1, above: o.above === true, centered: o.centered === true }
        ShellController.activePanel = "menu:" + id
    }
}
