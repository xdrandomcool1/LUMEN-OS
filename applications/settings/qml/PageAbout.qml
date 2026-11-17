import QtQuick
import Lumen.Design
import Lumen.Controls
import Lumen.Settings

SettingsPage {
    id: page
    readonly property var f: SystemInfo.facts
    function bytes(b) { return b >= 1e12 ? (b / 1e12).toFixed(1) + " TB" : (b / 1e9).toFixed(0) + " GB" }
    property string message
    Connections { target: SystemInfo; function onFailed(m) { page.message = m } }
    Column {
        width: parent.width; spacing: 6
        Symbol { anchors.horizontalCenter: parent.horizontalCenter; width: 72; height: 72; name: "lumen-logo"; weight: Symbol.Fill; color: Theme.accent }
        LText { anchors.horizontalCenter: parent.horizontalCenter; text: "LUMEN OS"; style: LText.LargeTitle }
        LText { anchors.horizontalCenter: parent.horizontalCenter; text: qsTr("Version %1").arg(page.f.lumenVersion); secondary: true }
    }
    FormSection {
        FormRow { label: qsTr("Name"); TextField { width: 220; text: SystemInfo.hostname; onEditingFinished: if (text !== SystemInfo.hostname) SystemInfo.setHostname(text) } }
        FormRow { label: qsTr("Model"); visible: (page.f.model || "").length > 0; LText { text: (page.f.vendor + " " + page.f.model).trim(); secondary: true } }
        FormRow { label: qsTr("Processor"); LText { text: page.f.cpu + " (" + page.f.cores + qsTr(" threads)"); secondary: true; width: 330; horizontalAlignment: Text.AlignRight } }
        FormRow { label: qsTr("Graphics"); LText { text: page.f.gpu; secondary: true; width: 330; horizontalAlignment: Text.AlignRight } }
        FormRow { label: qsTr("Memory"); LText { text: page.bytes(page.f.memory); secondary: true } }
        FormRow { label: qsTr("Storage"); LText { text: qsTr("%1 available of %2").arg(page.bytes(page.f.diskFree)).arg(page.bytes(page.f.diskTotal)); secondary: true } }
        FormRow { label: qsTr("Base system"); LText { text: page.f.os; secondary: true } }
        FormRow { label: qsTr("Kernel"); separator: false; LText { text: page.f.kernel; secondary: true } }
    }
    LText { visible: page.message.length > 0; text: page.message; color: Theme.red; width: parent.width; wrapMode: Text.Wrap }
    LText { width: parent.width; horizontalAlignment: Text.AlignHCenter; style: LText.Caption; tertiary: true; wrapMode: Text.Wrap
            text: qsTr("LUMEN OS is free software built on Fedora Linux, KDE Frameworks and Qt.") }
}
