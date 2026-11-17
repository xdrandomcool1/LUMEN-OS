// SPDX-License-Identifier: GPL-3.0-or-later
// System menus of the menu bar. Items: {text, shortcut, enabled, action,
// separator, checked, children}.
pragma Singleton
import QtQuick
import Lumen.Shell
import Lumen.System
import Lumen.Shell.Plasma

QtObject {
    function sep() { return { separator: true } }

    function lumenMenu() {
        return [
            { text: qsTr("About This Computer"), action: () => ShellController.openSettings("about") },
            sep(),
            { text: qsTr("System Settings…"), action: () => ShellController.openSettings("") },
            { text: qsTr("App Store…"), action: () => AppsModel.launch("lumen-store") },
            sep(),
            { text: qsTr("Force Quit…"), shortcut: "⌥⌘⎋", action: () => AppsModel.launch("lumen-activity-monitor") },
            sep(),
            { text: qsTr("Sleep"), action: () => ShellController.suspend() },
            { text: qsTr("Restart…"), action: () => ShellController.reboot() },
            { text: qsTr("Shut Down…"), action: () => ShellController.shutdown() },
            sep(),
            { text: qsTr("Lock Screen"), shortcut: "⌃⌘Q", action: () => ShellController.lockScreen() },
            { text: qsTr("Log Out…"), shortcut: "⇧⌘Q", action: () => ShellController.logout() }
        ]
    }

    function appMenu(name) {
        const tasks = ShellState.tasks
        const idx = tasks.activeTask
        const has = idx && idx.valid
        const items = [
            { text: qsTr("About %1").arg(name), enabled: has, action: () => {} },
            sep(),
            { text: qsTr("Hide %1").arg(name), shortcut: "⌘H", enabled: has, action: () => tasks.requestToggleMinimized(idx) },
            sep(),
            { text: qsTr("Quit %1").arg(name), shortcut: "⌘Q", enabled: has, action: () => tasks.requestClose(idx) }
        ]
        if (!has) {
            // Desktop focused: behave like the file manager's app menu.
            return [
                { text: qsTr("About Finder"), action: () => AppsModel.launch("lumen-finder") },
                sep(),
                { text: qsTr("Empty Trash…"), action: () => AppsModel.runCommand("gio trash --empty") },
                sep(),
                { text: qsTr("Show Desktop"), shortcut: "⌘D", action: () => ShellController.showDesktop() }
            ]
        }
        return items
    }

    function windowMenu() {
        const tasks = ShellState.tasks
        const idx = tasks.activeTask
        const has = idx && idx.valid
        return [
            { text: qsTr("Minimize"), shortcut: "⌘M", enabled: has, action: () => tasks.requestToggleMinimized(idx) },
            { text: qsTr("Zoom"), enabled: has, action: () => tasks.requestToggleMaximized(idx) },
            { text: qsTr("Enter Full Screen"), shortcut: "⌃⌘F", enabled: has, action: () => tasks.requestToggleFullScreen(idx) },
            sep(),
            { text: qsTr("Mission Control"), shortcut: "⌃↑", action: () => ShellController.showOverview() },
            { text: qsTr("Close Window"), shortcut: "⌘W", enabled: has, action: () => tasks.requestClose(idx) }
        ]
    }

    function batteryMenu() {
        const pct = Math.round(Battery.percentage)
        let state = Battery.charging ? qsTr("Charging") : (Battery.pluggedIn ? qsTr("Power Adapter") : qsTr("Battery"))
        return [
            { text: qsTr("Battery  %1%").arg(pct), enabled: false },
            { text: state, enabled: false },
            sep(),
            { text: qsTr("Battery Settings…"), action: () => ShellController.openSettings("power") }
        ]
    }
}
