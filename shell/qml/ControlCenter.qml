// SPDX-License-Identifier: GPL-3.0-or-later
// Control Center: connectivity, Focus, appearance, display, sound,
// keyboard, battery — every control is backed by a real system service.
import QtQuick
import Lumen.Design
import Lumen.Motion
import Lumen.Glass
import Lumen.Controls
import Lumen.Shell
import Lumen.Shell.Plasma
import org.kde.bluezqt as BluezQt

GlassSurface {
    id: cc
    property bool shown: false
    property string section: ""      // "", "wifi", "bluetooth", "sound"
    property string detail: section

    width: 324
    height: content.implicitHeight + 24
    radius: Theme.metrics.panelRadius
    material: Glass.Regular
    shadowScale: 1.1
    shadowOffsetY: 10

    onShownChanged: { if (shown) { detail = section; Network.scan(); Brightness.refresh() } }
    onSectionChanged: detail = section

    // Entrance: expands from the menu-bar trigger (top-right), modules
    // follow with a short stagger.
    Spring { id: appear; to: cc.shown ? 1 : 0; preset: Spring.Smooth }
    opacity: Math.min(1, appear.value * 1.4)
    visible: appear.value > 0.005
    transformOrigin: Item.TopRight
    scale: 0.92 + 0.08 * appear.value
    MouseArea { anchors.fill: parent; onPressed: m => m.accepted = true }

    NotificationSettings { id: notifSettings }
    readonly property bool focusOn: notifSettings.notificationsInhibitedUntil > new Date()
    function setFocus(on) {
        if (on) notifSettings.notificationsInhibitedUntil = new Date(Date.now() + 1000 * 3600 * 24 * 365 * 10)
        else notifSettings.notificationsInhibitedUntil = new Date(0)
        notifSettings.save()
    }
    readonly property var bt: BluezQt.Manager

    Column {
        id: content
        x: 12; y: 12
        width: cc.width - 24
        spacing: 12

        // ===== Main grid =====================================================
        Column {
            id: grid
            width: parent.width
            spacing: 12
            visible: cc.detail === ""
            readonly property real half: (width - 12) / 2

            Row {
                spacing: 12
                CCModule {
                    width: grid.half; height: 152
                    Column {
                        anchors.fill: parent; spacing: 13
                        CCToggle {
                            width: parent.width
                            symbol: Network.wifiEnabled ? "wifi-high" : "wifi-slash"
                            title: qsTr("Wi-Fi")
                            subtitle: !Network.wifiAvailable ? qsTr("No device") : !Network.wifiEnabled ? qsTr("Off")
                                      : Network.kind === "wifi" ? Network.connectionName : qsTr("Not Connected")
                            checked: Network.wifiEnabled && Network.wifiAvailable
                            showChevron: true
                            onToggled: Network.wifiEnabled = !Network.wifiEnabled
                            onDetailsRequested: cc.detail = "wifi"
                        }
                        CCToggle {
                            width: parent.width
                            symbol: "bluetooth"
                            title: qsTr("Bluetooth")
                            subtitle: !cc.bt.operational && cc.bt.adapters.length === 0 ? qsTr("No device")
                                      : cc.bt.bluetoothBlocked ? qsTr("Off") : qsTr("On")
                            checked: cc.bt.bluetoothOperational
                            showChevron: true
                            onToggled: cc.bt.bluetoothBlocked = !cc.bt.bluetoothBlocked
                            onDetailsRequested: cc.detail = "bluetooth"
                        }
                        CCToggle {
                            width: parent.width
                            symbol: Network.kind === "ethernet" ? "plugs-connected" : "network"
                            title: Network.kind === "ethernet" ? qsTr("Ethernet") : qsTr("Network")
                            subtitle: Network.ipAddress.length ? Network.ipAddress : qsTr("Offline")
                            checked: Network.kind !== "none"
                        }
                    }
                }
                Column {
                    spacing: 12
                    CCModule {
                        width: grid.half; height: 70
                        CCToggle {
                            anchors.verticalCenter: parent.verticalCenter; width: parent.width
                            symbol: "moon"; title: qsTr("Focus")
                            subtitle: cc.focusOn ? qsTr("Do Not Disturb") : ""
                            checked: cc.focusOn
                            onToggled: cc.setFocus(!cc.focusOn)
                        }
                    }
                    CCModule {
                        width: grid.half; height: 70
                        CCToggle {
                            anchors.verticalCenter: parent.verticalCenter; width: parent.width
                            symbol: Appearance.dark ? "moon-stars" : "sun"
                            title: qsTr("Dark Mode")
                            subtitle: Appearance.mode === Appearance.Auto ? qsTr("Automatic") : (Appearance.dark ? qsTr("On") : qsTr("Off"))
                            checked: Appearance.dark
                            onToggled: Appearance.mode = Appearance.dark ? Appearance.Light : Appearance.Dark
                        }
                    }
                }
            }

            CCModule {
                width: parent.width; height: 74
                visible: Brightness.available
                Column {
                    anchors.fill: parent; spacing: 8
                    LText { text: qsTr("Display"); style: LText.Callout; font.weight: Font.DemiBold }
                    CCSlider {
                        width: parent.width
                        symbol: "sun"
                        value: Brightness.value
                        onMoved: v => Brightness.value = v
                    }
                }
            }

            CCModule {
                width: parent.width; height: 74
                Column {
                    anchors.fill: parent; spacing: 8
                    Item {
                        width: parent.width; height: 16
                        LText { text: qsTr("Sound"); style: LText.Callout; font.weight: Font.DemiBold }
                        Symbol {
                            anchors.right: parent.right; width: 14; height: 14
                            name: "caret-right"; weight: Symbol.Bold; color: Theme.secondaryLabel
                            MouseArea { anchors.fill: parent; anchors.margins: -6; onClicked: cc.detail = "sound" }
                        }
                    }
                    CCSlider {
                        width: parent.width
                        symbol: Audio.symbol
                        enabledSlider: Audio.available
                        value: Audio.volume
                        onMoved: v => Audio.volume = v
                    }
                }
            }

            Row {
                spacing: 12
                Repeater {
                    model: [
                        { sym: "keyboard", label: KeyboardLayout.shortName.length ? KeyboardLayout.shortName : qsTr("Keyboard"), act: () => { if (KeyboardLayout.layouts.length > 1) KeyboardLayout.next(); else ShellController.openSettings("keyboard") } },
                        { sym: "selection", label: qsTr("Screenshot"), act: () => ShellController.takeScreenshot() },
                        { sym: "lock-simple", label: qsTr("Lock"), act: () => { ShellController.ClosePanels(); ShellController.lockScreen() } },
                        { sym: "gear-six", label: qsTr("Settings"), act: () => ShellController.openSettings("") }
                    ]
                    delegate: CCModule {
                        required property var modelData
                        width: (grid.width - 36) / 4; height: 70
                        Spring { id: pr; to: tap.pressed ? 1 : 0; preset: Spring.Snappy }
                        scale: 1 - 0.05 * pr.value
                        Column {
                            anchors.centerIn: parent; spacing: 5
                            Symbol { anchors.horizontalCenter: parent.horizontalCenter; width: 20; height: 20; name: modelData.sym; weight: Symbol.Fill; color: Theme.label }
                            LText { anchors.horizontalCenter: parent.horizontalCenter; text: modelData.label; style: LText.Caption2; width: Math.min(implicitWidth, 64) }
                        }
                        MouseArea { id: tap; anchors.fill: parent; anchors.margins: -12; onClicked: modelData.act() }
                    }
                }
            }

            CCModule {
                width: parent.width; height: 46
                visible: Battery.present
                Row {
                    anchors.verticalCenter: parent.verticalCenter; spacing: 8
                    Symbol { width: 22; height: 22; name: Battery.symbol; color: Battery.percentage < 15 && !Battery.charging ? Theme.red : Theme.label }
                    LText {
                        anchors.verticalCenter: parent.verticalCenter
                        text: Math.round(Battery.percentage) + "%  ·  " + (Battery.charging ? qsTr("Charging") : Battery.pluggedIn ? qsTr("Power Adapter") : qsTr("Battery"))
                        style: LText.Callout
                    }
                }
            }
        }

        // ===== Wi-Fi details ================================================
        CCDetail {
            visible: cc.detail === "wifi"
            width: parent.width
            title: qsTr("Wi-Fi")
            toggleChecked: Network.wifiEnabled
            onToggleClicked: Network.wifiEnabled = !Network.wifiEnabled
            onBack: cc.detail = ""
            settingsPage: "network"
            Column {
                width: parent.width
                LText { visible: Network.networks.length === 0; text: Network.wifiEnabled ? qsTr("Searching…") : qsTr("Wi-Fi is off"); secondary: true; topPadding: 6; bottomPadding: 6 }
                Repeater {
                    model: Network.networks
                    delegate: Column {
                        id: netRow
                        required property var modelData
                        width: parent.width
                        property bool asking: false
                        Item {
                            width: parent.width; height: 32
                            Rectangle { anchors.fill: parent; radius: 8; color: rowArea.containsMouse ? Theme.controlFill : "transparent" }
                            Rectangle {
                                x: 4; anchors.verticalCenter: parent.verticalCenter
                                width: 24; height: 24; radius: 12
                                color: netRow.modelData.active ? Theme.accent : (Theme.dark ? Qt.rgba(1, 1, 1, 0.14) : Qt.rgba(0, 0, 0, 0.08))
                                Symbol { anchors.centerIn: parent; width: 14; height: 14; weight: Symbol.Fill
                                         name: netRow.modelData.strength > 66 ? "wifi-high" : netRow.modelData.strength > 33 ? "wifi-medium" : "wifi-low"
                                         color: netRow.modelData.active ? "white" : Theme.label }
                            }
                            LText { x: 36; anchors.verticalCenter: parent.verticalCenter; text: netRow.modelData.ssid; width: parent.width - 70 }
                            Symbol { visible: netRow.modelData.secure; anchors.right: parent.right; anchors.rightMargin: 8; anchors.verticalCenter: parent.verticalCenter
                                     width: 13; height: 13; name: "lock-simple"; weight: Symbol.Fill; color: Theme.secondaryLabel }
                            MouseArea {
                                id: rowArea; anchors.fill: parent; hoverEnabled: true
                                onClicked: {
                                    if (netRow.modelData.active) return
                                    if (netRow.modelData.secure && !netRow.modelData.known) netRow.asking = !netRow.asking
                                    else Network.connectTo(netRow.modelData.ssid)
                                }
                            }
                        }
                        Row {
                            visible: netRow.asking
                            spacing: 6; leftPadding: 36; bottomPadding: 6
                            TextField { id: pw; width: netRow.width - 110; echoMode: TextInput.Password; placeholderText: qsTr("Password")
                                        onAccepted: { Network.connectTo(netRow.modelData.ssid, text); netRow.asking = false } }
                            Button { text: qsTr("Join"); kind: Button.Prominent; onClicked: { Network.connectTo(netRow.modelData.ssid, pw.text); netRow.asking = false } }
                        }
                    }
                }
                LText { visible: Network.lastError.length > 0; text: Network.lastError; color: Theme.red; style: LText.Caption; width: parent.width; wrapMode: Text.Wrap }
            }
        }

        // ===== Bluetooth details ============================================
        CCDetail {
            visible: cc.detail === "bluetooth"
            width: parent.width
            title: qsTr("Bluetooth")
            toggleChecked: cc.bt.bluetoothOperational
            onToggleClicked: cc.bt.bluetoothBlocked = !cc.bt.bluetoothBlocked
            onBack: cc.detail = ""
            settingsPage: "bluetooth"
            Column {
                width: parent.width
                LText { visible: devices.count === 0; text: qsTr("No devices"); secondary: true; topPadding: 6; bottomPadding: 6 }
                Repeater {
                    model: BluezQt.DevicesModel { id: devices }
                    delegate: Item {
                        required property var model
                        width: parent.width; height: 32
                        Rectangle { anchors.fill: parent; radius: 8; color: devArea.containsMouse ? Theme.controlFill : "transparent" }
                        Rectangle {
                            x: 4; anchors.verticalCenter: parent.verticalCenter; width: 24; height: 24; radius: 12
                            color: model.Connected ? Theme.accent : (Theme.dark ? Qt.rgba(1, 1, 1, 0.14) : Qt.rgba(0, 0, 0, 0.08))
                            Symbol { anchors.centerIn: parent; width: 14; height: 14; name: "bluetooth"; weight: Symbol.Fill; color: model.Connected ? "white" : Theme.label }
                        }
                        LText { x: 36; anchors.verticalCenter: parent.verticalCenter; text: model.Name; width: parent.width - 100 }
                        LText { anchors.right: parent.right; anchors.rightMargin: 8; anchors.verticalCenter: parent.verticalCenter
                                text: model.Connected ? qsTr("Connected") : ""; secondary: true; style: LText.Caption }
                        MouseArea { id: devArea; anchors.fill: parent; hoverEnabled: true
                            onClicked: { if (model.Connected) model.Device.disconnectFromDevice(); else model.Device.connectToDevice() } }
                    }
                }
            }
        }

        // ===== Sound output details =========================================
        CCDetail {
            visible: cc.detail === "sound"
            width: parent.width
            title: qsTr("Sound")
            showToggle: false
            onBack: cc.detail = ""
            settingsPage: "sound"
            Column {
                width: parent.width
                spacing: 8
                CCSlider { width: parent.width; symbol: Audio.symbol; value: Audio.volume; enabledSlider: Audio.available; onMoved: v => Audio.volume = v }
                LText { text: qsTr("Output"); style: LText.Caption; secondary: true; topPadding: 4 }
                Repeater {
                    model: Audio.outputs
                    delegate: Item {
                        required property var modelData
                        width: parent.width; height: 30
                        Rectangle { anchors.fill: parent; radius: 8; color: outArea.containsMouse ? Theme.controlFill : "transparent" }
                        Symbol { x: 6; anchors.verticalCenter: parent.verticalCenter; width: 16; height: 16; name: "speaker-high"; weight: Symbol.Fill; color: Theme.label }
                        LText { x: 30; anchors.verticalCenter: parent.verticalCenter; text: modelData.description; width: parent.width - 60 }
                        Symbol { visible: modelData.active; anchors.right: parent.right; anchors.rightMargin: 8; anchors.verticalCenter: parent.verticalCenter
                                 width: 13; height: 13; name: "check"; weight: Symbol.Bold; color: Theme.accent }
                        MouseArea { id: outArea; anchors.fill: parent; hoverEnabled: true; onClicked: Audio.setDefaultOutput(modelData.name) }
                    }
                }
                LText { visible: !Audio.available; text: qsTr("No output devices"); secondary: true }
            }
        }
    }
}
