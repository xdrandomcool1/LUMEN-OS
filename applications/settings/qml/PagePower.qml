import QtQuick
import Lumen.Design
import Lumen.Controls
import Lumen.System
import Lumen.Settings

SettingsPage {
    heading: qsTr("Battery"); symbol: Battery.present ? Battery.symbol : "plug"; tint: Theme.green
    summary: Battery.present ? qsTr("%1% — %2").arg(Math.round(Battery.percentage)).arg(Battery.charging ? qsTr("charging") : Battery.pluggedIn ? qsTr("power adapter") : qsTr("on battery"))
                             : qsTr("This computer runs on mains power.")
    FormSection {
        title: qsTr("Energy mode")
        visible: PowerProfiles.available
        FormRow { label: qsTr("Mode"); separator: false
            SegmentedControl { width: 300; model: [qsTr("Low Power"), qsTr("Balanced"), qsTr("Performance")]
                readonly property var ids: ["power-saver", "balanced", "performance"]
                currentIndex: Math.max(0, ids.indexOf(PowerProfiles.active))
                onActivated: i => PowerProfiles.active = ids[i] } }
    }
    FormSection {
        title: qsTr("When idle")
        FormRow { label: qsTr("Turn display off after")
            PopUpButton { width: 150; readonly property var secs: [60, 120, 300, 600, 1800, -1]
                model: [qsTr("1 minute"), qsTr("2 minutes"), qsTr("5 minutes"), qsTr("10 minutes"), qsTr("30 minutes"), qsTr("Never")]
                currentIndex: Math.max(0, secs.indexOf(Number(KdeConfig.read("powerdevilrc", "AC/Display", "TurnOffDisplayIdleTimeoutSec", 600))))
                onActivated: i => { KdeConfig.write("powerdevilrc", "AC/Display", "TurnOffDisplayIdleTimeoutSec", secs[i]); KdeConfig.write("powerdevilrc", "AC/Display", "TurnOffDisplayWhenIdle", secs[i] > 0); KdeConfig.reloadPowerDevil() } } }
        FormRow { label: qsTr("Sleep after"); separator: false
            PopUpButton { width: 150; readonly property var secs: [600, 900, 1800, 3600, -1]
                model: [qsTr("10 minutes"), qsTr("15 minutes"), qsTr("30 minutes"), qsTr("1 hour"), qsTr("Never")]
                currentIndex: Math.max(0, secs.indexOf(Number(KdeConfig.read("powerdevilrc", "AC/SuspendAndShutdown", "AutoSuspendIdleTimeoutSec", 900))))
                onActivated: i => { KdeConfig.write("powerdevilrc", "AC/SuspendAndShutdown", "AutoSuspendIdleTimeoutSec", Math.max(secs[i], 0)); KdeConfig.write("powerdevilrc", "AC/SuspendAndShutdown", "AutoSuspendAction", secs[i] > 0 ? 1 : 0); KdeConfig.reloadPowerDevil() } } }
    }
}
