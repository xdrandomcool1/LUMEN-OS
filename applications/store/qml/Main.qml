// SPDX-License-Identifier: GPL-3.0-or-later
// App Store — discover, install and update Flatpak applications.
import QtQuick
import Lumen.Design
import Lumen.Motion
import Lumen.Glass
import Lumen.Controls
import Lumen.Store

AppWindow {
    id: win
    title: page === "discover" ? qsTr("Discover") : page === "installed" ? qsTr("Installed") : qsTr("Updates")
    width: 1000; height: 680
    visible: true
    property string page: "discover"
    property string message
    property string query

    // Popular, freely licensed apps on Flathub (real application IDs).
    readonly property var featured: [
        { appId: "org.mozilla.firefox", name: "Firefox", blurb: qsTr("Fast, private web browser") },
        { appId: "org.libreoffice.LibreOffice", name: "LibreOffice", blurb: qsTr("Office suite") },
        { appId: "org.gimp.GIMP", name: "GIMP", blurb: qsTr("Image editor") },
        { appId: "org.inkscape.Inkscape", name: "Inkscape", blurb: qsTr("Vector graphics") },
        { appId: "org.blender.Blender", name: "Blender", blurb: qsTr("3D creation suite") },
        { appId: "org.videolan.VLC", name: "VLC", blurb: qsTr("Media player") },
        { appId: "org.kde.krita", name: "Krita", blurb: qsTr("Digital painting") },
        { appId: "com.obsproject.Studio", name: "OBS Studio", blurb: qsTr("Recording & streaming") },
        { appId: "org.audacityteam.Audacity", name: "Audacity", blurb: qsTr("Audio editor") },
        { appId: "org.signal.Signal", name: "Signal", blurb: qsTr("Private messenger") },
        { appId: "com.github.tchx84.Flatseal", name: "Flatseal", blurb: qsTr("Manage app permissions") },
        { appId: "org.kde.kdenlive", name: "Kdenlive", blurb: qsTr("Video editor") }
    ]

    Connections { target: Flatpak; function onOperationFinished(id, ok, msg) { win.message = ok ? "" : msg } }
    Component.onCompleted: Flatpak.refreshUpdates()

    sidebar: Column {
        spacing: 2
        SearchField { id: search; x: 10; width: parent.width - 20; onTextChanged: { win.query = text; win.page = "discover"; searchTimer.restart() } }
        Timer { id: searchTimer; interval: 350; onTriggered: Flatpak.search(win.query) }
        Item { width: 1; height: 8 }
        Repeater {
            model: [["discover", qsTr("Discover"), "star"], ["installed", qsTr("Installed"), "package"], ["updates", qsTr("Updates"), "arrow-clockwise"]]
            SidebarItem { required property var modelData; x: 8; width: parent.width - 16; text: modelData[1]; symbol: modelData[2]
                          selected: win.page === modelData[0]; badge: modelData[0] === "updates" && Flatpak.updates.length ? String(Flatpak.updates.length) : ""
                          onClicked: win.page = modelData[0] }
        }
    }

    component AppCard: Rectangle {
        id: card
        property var app
        readonly property bool installed: Flatpak.isInstalled(app.appId) || app.installed === true
        readonly property bool busy: Flatpak.busyApp === app.appId
        width: 300; height: 76; radius: 14
        color: Theme.dark ? Qt.rgba(1, 1, 1, 0.05) : "white"
        border.width: 0.5; border.color: Theme.controlBorder
        Image {
            id: icon
            x: 12; anchors.verticalCenter: parent.verticalCenter; width: 52; height: 52
            source: card.app.icon || Flatpak.iconFor(card.app.appId); sourceSize: Qt.size(104, 104); asynchronous: true
            Rectangle { anchors.fill: parent; radius: 12; color: Theme.controlFillPressed; visible: icon.status !== Image.Ready
                Symbol { anchors.centerIn: parent; width: 26; height: 26; name: "package"; color: Theme.secondaryLabel } }
        }
        Column {
            x: 74; anchors.verticalCenter: parent.verticalCenter; width: parent.width - 74 - 92
            LText { width: parent.width; text: card.app.name; style: LText.Headline }
            LText { width: parent.width; text: card.app.blurb || card.app.description || card.app.version || ""; secondary: true; style: LText.Callout }
        }
        Item {
            anchors.right: parent.right; anchors.rightMargin: 12; anchors.verticalCenter: parent.verticalCenter
            width: 76; height: 26
            Button { anchors.fill: parent; visible: !card.busy; size: Button.Small; kind: card.installed ? 0 /* Button.Bordered */ : 1 /* Button.Prominent */
                     text: card.installed ? qsTr("Open") : qsTr("Get")
                     onClicked: card.installed ? Flatpak.launch(card.app.appId) : Flatpak.install(card.app.appId) }
            ProgressBar { anchors.centerIn: parent; width: 70; visible: card.busy; value: Flatpak.progress / 100; indeterminate: Flatpak.progress === 0 }
        }
    }

    Flickable {
        x: 0; y: win.toolbarHeight
        width: win.contentArea.width; height: win.contentArea.height - y
        contentHeight: col.implicitHeight + 40
        clip: true
        ScrollBar.vertical: ScrollBar {}
        Column {
            id: col
            x: 24; width: parent.width - 48; spacing: 18
            LText { visible: !Flatpak.available; text: qsTr("Flatpak is not installed on this system."); color: Theme.red }
            LText { visible: win.message.length > 0; text: win.message; color: Theme.red; wrapMode: Text.Wrap; width: parent.width }

            // Discover
            Column {
                visible: win.page === "discover"
                width: parent.width; spacing: 14
                LText { visible: win.query.length < 2; text: qsTr("Featured"); style: LText.Title2 }
                Flow {
                    width: parent.width; spacing: 14
                    Repeater { model: win.query.length >= 2 ? Flatpak.results : win.featured; AppCard { required property var modelData; app: modelData } }
                }
                Row { visible: Flatpak.searching; spacing: 8; Spinner {} LText { text: qsTr("Searching Flathub…"); secondary: true } }
                LText { visible: win.query.length >= 2 && !Flatpak.searching && Flatpak.results.length === 0; text: qsTr("No results"); tertiary: true; style: LText.Title3 }
            }
            // Installed
            Column {
                visible: win.page === "installed"
                width: parent.width; spacing: 14
                LText { text: qsTr("%n installed app(s)", "", Flatpak.installed.length); style: LText.Title2 }
                Repeater {
                    model: Flatpak.installed
                    Rectangle {
                        required property var modelData
                        width: col.width; height: 56; radius: 12
                        color: Theme.dark ? Qt.rgba(1, 1, 1, 0.04) : "white"; border.width: 0.5; border.color: Theme.controlBorder
                        Image { x: 12; anchors.verticalCenter: parent.verticalCenter; width: 36; height: 36; source: modelData.icon; sourceSize: Qt.size(72, 72) }
                        Column { x: 60; anchors.verticalCenter: parent.verticalCenter
                            LText { text: modelData.name; style: LText.Headline }
                            LText { text: modelData.appId + "  ·  " + modelData.version + "  ·  " + modelData.size; secondary: true; style: LText.Caption } }
                        Row { anchors.right: parent.right; anchors.rightMargin: 12; anchors.verticalCenter: parent.verticalCenter; spacing: 8
                            Button { size: Button.Small; text: qsTr("Open"); onClicked: Flatpak.launch(modelData.appId) }
                            Button { size: Button.Small; kind: Button.Destructive; text: qsTr("Remove"); enabled: Flatpak.busyApp === ""; onClicked: Flatpak.uninstall(modelData.appId) } }
                    }
                }
            }
            // Updates
            Column {
                visible: win.page === "updates"
                width: parent.width; spacing: 14
                Row { spacing: 12
                    LText { text: Flatpak.updates.length ? qsTr("%n update(s) available", "", Flatpak.updates.length) : qsTr("All apps are up to date"); style: LText.Title2 }
                    Button { visible: Flatpak.updates.length > 0; kind: Button.Prominent; text: qsTr("Update All"); enabled: Flatpak.busyApp === ""; onClicked: Flatpak.updateAll() } }
                ProgressBar { visible: Flatpak.busyApp === "*"; width: parent.width; value: Flatpak.progress / 100 }
                Repeater { model: Flatpak.updates
                    FormRow { required property var modelData; width: col.width; label: modelData.name; detail: modelData.appId + " → " + modelData.version } }
            }
        }
    }
}
