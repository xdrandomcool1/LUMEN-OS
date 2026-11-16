import QtQuick
import Lumen.Design
import Lumen.Controls

SettingsPage {
    heading: qsTr("Accessibility"); symbol: "person-arms-spread"; tint: Theme.blue
    summary: qsTr("Adapt the interface to your vision and motion preferences.")
    FormSection {
        title: qsTr("Display")
        FormRow { label: qsTr("Reduce motion"); detail: qsTr("Replace movement with quick fades"); Toggle { checked: Appearance.reduceMotion; onToggled: Appearance.reduceMotion = checked } }
        FormRow { label: qsTr("Reduce transparency"); detail: qsTr("Use opaque materials instead of glass"); Toggle { checked: Appearance.reduceTransparency; onToggled: Appearance.reduceTransparency = checked } }
        FormRow { label: qsTr("Increase contrast"); detail: qsTr("Stronger borders and less translucent surfaces"); separator: false; Toggle { checked: Appearance.increaseContrast; onToggled: Appearance.increaseContrast = checked } }
    }
}
