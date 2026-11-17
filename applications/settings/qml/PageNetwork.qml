import QtQuick
import Lumen.Design
import Lumen.Controls
import Lumen.System

SettingsPage {
    heading: qsTr("Network"); symbol: "wifi-high"; tint: Theme.blue
    summary: Network.kind === "none" ? qsTr("Not connected") : qsTr("Connected to %1").arg(Network.connectionName)
    Component.onCompleted: Network.scan()
    FormSection {
        FormRow { label: qsTr("Wi-Fi"); symbol: "wifi-high"; enabled: Network.wifiAvailable
                  detail: !Network.wifiAvailable ? qsTr("No Wi-Fi hardware") : ""
                  Toggle { checked: Network.wifiEnabled; onToggled: Network.wifiEnabled = checked } }
        FormRow { label: qsTr("IP address"); symbol: "globe"; tint: Theme.gray; separator: false; LText { text: Network.ipAddress || "—"; secondary: true } }
    }
    FormSection {
        title: qsTr("Known and nearby networks")
        visible: Network.wifiEnabled && Network.wifiAvailable
        Repeater {
            model: Network.networks
            FormRow {
                required property var modelData
                required property int index
                label: modelData.ssid
                detail: modelData.active ? qsTr("Connected") : modelData.known ? qsTr("Saved") : ""
                separator: index < Network.networks.length - 1
                Row {
                    spacing: 8
                    Symbol { visible: modelData.secure; width: 13; height: 13; name: "lock-simple"; weight: Symbol.Fill; color: Theme.secondaryLabel; anchors.verticalCenter: parent.verticalCenter }
                    Symbol { width: 16; height: 16; name: modelData.strength > 66 ? "wifi-high" : modelData.strength > 33 ? "wifi-medium" : "wifi-low"; color: Theme.label; anchors.verticalCenter: parent.verticalCenter }
                    Button { text: modelData.active ? qsTr("Disconnect") : qsTr("Connect"); size: Button.Small
                             onClicked: modelData.active ? Network.disconnectWifi() : (modelData.secure && !modelData.known ? pw.open(modelData.ssid) : Network.connectTo(modelData.ssid)) }
                }
            }
        }
    }
    FormSection {
        id: pw
        property string ssid
        function open(s) { ssid = s; visible = true; field.forceActiveFocus() }
        visible: false
        title: qsTr("Join “%1”").arg(ssid)
        FormRow { label: qsTr("Password"); separator: false
            Row { spacing: 6
                TextField { id: field; width: 200; echoMode: TextInput.Password; onAccepted: { Network.connectTo(pw.ssid, text); pw.visible = false } }
                Button { text: qsTr("Join"); kind: Button.Prominent; onClicked: { Network.connectTo(pw.ssid, field.text); pw.visible = false } } } }
    }
    LText { visible: Network.lastError.length > 0; text: Network.lastError; color: Theme.red; width: parent.width; wrapMode: Text.Wrap }
}
