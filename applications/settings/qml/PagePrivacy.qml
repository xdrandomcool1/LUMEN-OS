import QtQuick
import Lumen.Design
import Lumen.Controls
import Lumen.Settings

SettingsPage {
    id: page
    heading: qsTr("Privacy & Security"); symbol: "shield-check"; tint: Theme.blue
    summary: qsTr("Control what can reach this computer and when it locks.")
    property string message
    Connections { target: Firewall; function onFailed(m) { page.message = m } }
    FormSection {
        title: qsTr("Firewall")
        footer: Firewall.running ? qsTr("Incoming connections are filtered (zone: %1).").arg(Firewall.zone) : qsTr("All incoming connections are allowed.")
        FormRow { label: qsTr("Firewall"); separator: false; enabled: Firewall.available
                  detail: Firewall.available ? "" : qsTr("firewalld is not installed")
                  Toggle { checked: Firewall.running; onToggled: Firewall.setRunning(checked) } }
    }
    FormSection {
        title: qsTr("Lock Screen")
        FormRow { label: qsTr("Lock automatically")
                  Toggle { checked: KdeConfig.read("kscreenlockerrc", "Daemon", "Autolock", true); onToggled: { KdeConfig.write("kscreenlockerrc", "Daemon", "Autolock", checked); KdeConfig.reloadScreenLocker() } } }
        FormRow { label: qsTr("Require password after")
            PopUpButton { width: 150; readonly property var mins: [1, 5, 10, 15, 30]
                model: [qsTr("1 minute"), qsTr("5 minutes"), qsTr("10 minutes"), qsTr("15 minutes"), qsTr("30 minutes")]
                currentIndex: Math.max(0, mins.indexOf(Number(KdeConfig.read("kscreenlockerrc", "Daemon", "Timeout", 5))))
                onActivated: i => { KdeConfig.write("kscreenlockerrc", "Daemon", "Timeout", mins[i]); KdeConfig.reloadScreenLocker() } } }
        FormRow { label: qsTr("Lock after waking from sleep"); separator: false
                  Toggle { checked: KdeConfig.read("kscreenlockerrc", "Daemon", "LockOnResume", true); onToggled: { KdeConfig.write("kscreenlockerrc", "Daemon", "LockOnResume", checked); KdeConfig.reloadScreenLocker() } } }
    }
    FormSection {
        title: qsTr("Applications")
        footer: qsTr("Flatpak applications run sandboxed and must ask through portals to access your files, camera, screen and location.")
        FormRow { label: qsTr("Sandboxed apps"); separator: false; LText { text: qsTr("Managed in App Store"); secondary: true } }
    }
    LText { visible: page.message.length > 0; text: page.message; color: Theme.red; width: parent.width; wrapMode: Text.Wrap }
}
