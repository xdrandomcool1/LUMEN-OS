// SPDX-License-Identifier: GPL-3.0-or-later
// Spotlight: global search & launcher (Super+Space).
import QtQuick
import Lumen.Design
import Lumen.Motion
import Lumen.Glass
import Lumen.Controls
import Lumen.Shell
import Lumen.System
import "calc.js" as Calc

Item {
    id: sp
    property bool shown: false
    property BackdropBlur backdrop: null
    property bool compositorBlur: false
    property int selected: 0
    property string initialQuery: ""
    onInitialQueryChanged: if (shown && initialQuery.length) { field.text = initialQuery; field.cursorPosition = field.text.length }
    readonly property string query: field.text

    width: 680
    height: field.height + (results.count > 0 ? resultsGlass.height + 10 : 0)

    Spring { id: appear; to: sp.shown ? 1 : 0; preset: Spring.Snappy }
    visible: appear.value > 0.005
    opacity: Math.min(1, appear.value * 1.3)
    scale: 0.94 + 0.06 * appear.value
    transformOrigin: Item.Top

    onShownChanged: {
        if (shown) { field.text = initialQuery; field.forceActiveFocus() }
    }

    // ---- Result sources ----------------------------------------------------
    readonly property var settingsPages: [
        { k: "appearance dark light mode accent wallpaper", t: qsTr("Appearance"), p: "appearance", i: "palette" },
        { k: "wallpaper background desktop picture", t: qsTr("Wallpaper"), p: "wallpaper", i: "image" },
        { k: "display screen resolution scale brightness night", t: qsTr("Displays"), p: "display", i: "monitor" },
        { k: "wifi wi-fi network internet ethernet vpn", t: qsTr("Network"), p: "network", i: "wifi-high" },
        { k: "bluetooth devices headphones", t: qsTr("Bluetooth"), p: "bluetooth", i: "bluetooth" },
        { k: "sound audio volume speaker microphone", t: qsTr("Sound"), p: "sound", i: "speaker-high" },
        { k: "keyboard layout shortcuts typing", t: qsTr("Keyboard"), p: "keyboard", i: "keyboard" },
        { k: "mouse trackpad touchpad pointer gestures", t: qsTr("Mouse & Trackpad"), p: "mouse", i: "mouse-simple" },
        { k: "power battery energy sleep", t: qsTr("Battery"), p: "power", i: "battery-high" },
        { k: "users accounts password login avatar", t: qsTr("Users & Groups"), p: "users", i: "users" },
        { k: "privacy security firewall location permissions", t: qsTr("Privacy & Security"), p: "privacy", i: "shield-check" },
        { k: "dock menu bar magnification", t: qsTr("Desktop & Dock"), p: "dock", i: "app-window" },
        { k: "accessibility reduce motion transparency contrast", t: qsTr("Accessibility"), p: "accessibility", i: "person-arms-spread" },
        { k: "about computer system version hardware", t: qsTr("About"), p: "about", i: "info" }
    ]
    readonly property var actions: [
        { k: "lock screen", t: qsTr("Lock Screen"), i: "lock-simple", a: () => ShellController.lockScreen() },
        { k: "sleep suspend", t: qsTr("Sleep"), i: "moon", a: () => ShellController.suspend() },
        { k: "restart reboot", t: qsTr("Restart…"), i: "arrow-clockwise", a: () => ShellController.reboot() },
        { k: "shut down power off shutdown", t: qsTr("Shut Down…"), i: "power", a: () => ShellController.shutdown() },
        { k: "log out logout sign out", t: qsTr("Log Out…"), i: "sign-out", a: () => ShellController.logout() },
        { k: "empty trash", t: qsTr("Empty Trash"), i: "trash", a: () => AppsModel.emptyTrash() },
        { k: "dark mode toggle appearance", t: Appearance.dark ? qsTr("Turn Dark Mode Off") : qsTr("Turn Dark Mode On"), i: "circle-half",
          a: () => Appearance.mode = Appearance.dark ? Appearance.Light : Appearance.Dark },
        { k: "screenshot screen capture", t: qsTr("Take Screenshot"), i: "selection", a: () => ShellController.takeScreenshot() },
        { k: "mission control overview windows", t: qsTr("Mission Control"), i: "squares-four", a: () => ShellController.showOverview() },
        { k: "show desktop", t: qsTr("Show Desktop"), i: "desktop", a: () => ShellController.showDesktop() },
        { k: "nvidia graphics driver gpu install", t: qsTr("Install NVIDIA Driver"), i: "cpu", when: () => SystemActions.needsNvidiaDriver,
          a: () => SystemActions.installNvidiaDriver() }
    ]

    function matches(keywords, title, q) {
        q = q.toLowerCase()
        if (title.toLowerCase().startsWith(q)) return 2
        const words = (keywords + " " + title.toLowerCase()).split(" ")
        for (let w of words) if (w.startsWith(q)) return 1
        return 0
    }

    readonly property var model: {
        const q = query.trim()
        if (!q.length) return []
        const out = []
        const calc = Calc.evaluate(q)
        if (calc !== null)
            out.push({ section: qsTr("Calculator"), title: Calc.format(calc), subtitle: q + " =", symbol: "calculator",
                       run: () => { field.text = String(calc); field.cursorPosition = field.text.length }, keep: true })
        const apps = AppsModel.search(q, 8)
        for (let i = 0; i < apps.length; ++i) {
            const a = apps[i]
            out.push({ section: i === 0 && calc === null ? qsTr("Top Hit") : qsTr("Applications"), title: a.name,
                       subtitle: a.genericName || a.comment || "", icon: a.iconName, run: () => AppsModel.launch(a.desktopId) })
        }
        for (const s of settingsPages)
            if (matches(s.k, s.t, q)) out.push({ section: qsTr("System Settings"), title: s.t, subtitle: qsTr("Settings"), symbol: s.i, run: () => ShellController.openSettings(s.p) })
        for (const a of actions)
            if ((!a.when || a.when()) && matches(a.k, a.t, q)) out.push({ section: qsTr("Actions"), title: a.t, subtitle: "", symbol: a.i, run: a.a })
        const files = FileSearch.search(q, 8)
        for (const f of files)
            out.push({ section: qsTr("Documents"), title: f.name, subtitle: f.folder, icon: f.iconName, run: () => AppsModel.openUrl(f.url) })
        return out
    }

    function activate(i) {
        const r = model[i]
        if (!r) return
        r.run()
        if (!r.keep) ShellController.ClosePanels()
    }

    // ---- Search field ------------------------------------------------------
    GlassSurface {
        id: fieldGlass
        width: parent.width
        height: 56
        radius: height / 2
        material: Glass.Floating
        backdrop: sp.backdrop
        compositorBlur: sp.compositorBlur
        tintAlpha: sp.backdrop ? -1 : 0.55
        shadowScale: 1.1
        shadowOffsetY: 10

        Symbol {
            x: 20; anchors.verticalCenter: parent.verticalCenter
            width: 22; height: 22; name: "magnifying-glass"; weight: Symbol.Bold; color: Theme.secondaryLabel
        }
        TextInput {
            id: field
            x: 54; width: parent.width - 74
            anchors.verticalCenter: parent.verticalCenter
            font.family: Theme.fontFamily; font.pixelSize: 22; font.weight: Font.Normal
            color: Theme.label
            selectionColor: Theme.selection
            selectedTextColor: "white"
            clip: true
            focus: true
            Text {
                anchors.fill: parent
                visible: !field.text.length
                text: qsTr("Spotlight Search")
                font: field.font
                color: Theme.tertiaryLabel
                renderType: Text.QtRendering
            }
            Keys.onDownPressed: sp.selected = Math.min(sp.model.length - 1, sp.selected + 1)
            Keys.onUpPressed: sp.selected = Math.max(0, sp.selected - 1)
            Keys.onReturnPressed: sp.activate(sp.selected)
            Keys.onEnterPressed: sp.activate(sp.selected)
            Keys.onEscapePressed: ShellController.ClosePanels()
            Keys.onTabPressed: sp.selected = (sp.selected + 1) % Math.max(1, sp.model.length)
        }
    }

    // ---- Results -----------------------------------------------------------
    GlassSurface {
        id: resultsGlass
        y: fieldGlass.height + 10
        width: parent.width
        height: Math.min(440, list.contentHeight + 16)
        visible: results.count > 0
        radius: Theme.metrics.panelRadius
        material: Glass.Frosted
        backdrop: sp.backdrop
        compositorBlur: sp.compositorBlur
        shadowScale: 1.0
        shadowOffsetY: 10

        ListView {
            id: list
            anchors.fill: parent
            anchors.margins: 8
            clip: true
            model: ListModel { id: results }
            currentIndex: sp.selected
            highlightMoveDuration: Motion.fast
            highlightFollowsCurrentItem: true
            boundsBehavior: Flickable.StopAtBounds
            section.property: "section"
            section.delegate: LText {
                required property string section
                text: section; style: LText.Caption; font.weight: Font.DemiBold; secondary: true
                leftPadding: 10; topPadding: 6; bottomPadding: 2
            }
            highlight: Rectangle { radius: Theme.metrics.hoverRadius; color: Theme.selection }
            add: Transition { NumberAnimation { properties: "opacity"; from: 0; to: 1; duration: Motion.fast } }
            delegate: Item {
                id: res
                required property int index
                required property string title
                required property string subtitle
                required property string icon
                required property string symbol
                readonly property bool current: ListView.isCurrentItem
                width: list.width
                height: 40
                AppIcon {
                    visible: res.icon.length > 0
                    x: 10; anchors.verticalCenter: parent.verticalCenter
                    size: 28; source: res.icon
                }
                Rectangle {
                    visible: res.icon.length === 0
                    x: 10; anchors.verticalCenter: parent.verticalCenter
                    width: 28; height: 28; radius: width / 4
                    color: res.current ? Qt.rgba(1, 1, 1, 0.25) : Theme.controlFillPressed
                    Symbol { anchors.centerIn: parent; width: 17; height: 17; name: res.symbol; weight: Symbol.Fill; color: res.current ? "white" : Theme.label }
                }
                Column {
                    x: 48; anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - 60
                    LText { width: parent.width; text: res.title; color: res.current ? "white" : Theme.label; font.weight: Font.Medium }
                    LText { width: parent.width; visible: res.subtitle.length > 0; text: res.subtitle; style: LText.Caption
                            color: res.current ? Qt.rgba(1, 1, 1, 0.8) : Theme.secondaryLabel }
                }
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: sp.selected = res.index
                    onClicked: sp.activate(res.index)
                }
            }
        }
    }

    // Results are mirrored into a ListModel for sectioning and add transitions.
    onModelChanged: {
        selected = 0
        results.clear()
        for (const r of model)
            results.append({ section: r.section, title: r.title, subtitle: r.subtitle || "", icon: r.icon || "", symbol: r.symbol || "" })
    }
}
