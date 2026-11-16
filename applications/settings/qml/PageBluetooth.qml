import QtQuick
import Lumen.Design
import Lumen.Controls
import org.kde.bluezqt as BluezQt

SettingsPage {
    id: page
    readonly property var bt: BluezQt.Manager
    readonly property var adapter: bt.usableAdapter
    heading: qsTr("Bluetooth"); symbol: "bluetooth"; tint: Theme.blue
    summary: !bt.operational ? qsTr("Bluetooth service is not running.") : bt.adapters.length === 0 ? qsTr("No Bluetooth hardware was found.")
             : qsTr("This computer is discoverable as “%1” while Bluetooth settings are open.").arg(adapter ? adapter.name : "")
    FormSection {
        FormRow { label: qsTr("Bluetooth"); symbol: "bluetooth"; separator: false; enabled: bt.adapters.length > 0
                  Toggle { checked: bt.bluetoothOperational; onToggled: bt.bluetoothBlocked = !checked } }
    }
    FormSection {
        title: qsTr("Devices")
        visible: bt.bluetoothOperational
        Repeater {
            model: BluezQt.DevicesModel { id: devices }
            FormRow {
                required property var model
                label: model.Name
                detail: model.Connected ? qsTr("Connected") : model.Paired ? qsTr("Not connected") : qsTr("Not paired")
                Button { size: Button.Small; text: model.Connected ? qsTr("Disconnect") : qsTr("Connect")
                         onClicked: model.Connected ? model.Device.disconnectFromDevice() : model.Device.connectToDevice() }
            }
        }
        FormRow { visible: devices.count === 0; label: qsTr("Searching for devices…"); separator: false; Spinner {} }
    }
    Component.onCompleted: if (adapter && adapter.powered) { adapter.discoverable = true; adapter.startDiscovery() }
    Component.onDestruction: if (adapter && adapter.discovering) adapter.stopDiscovery()
}
