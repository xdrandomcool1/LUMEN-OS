import QtQuick
import Lumen.Design
import Lumen.Controls
import Lumen.Settings

SettingsPage {
    heading: qsTr("Displays"); symbol: "monitor"; tint: Theme.blue
    summary: Displays.ready ? qsTr("%n display(s) connected", "", Displays.outputs.length) : qsTr("Reading display configuration…")
    Repeater {
        model: Displays.outputs
        FormSection {
            required property var modelData
            title: modelData.name + (modelData.internal ? qsTr(" (built-in)") : "")
            FormRow { label: qsTr("Use this display"); visible: Displays.outputs.length > 1
                      Toggle { checked: modelData.enabled; onToggled: Displays.setEnabled(modelData.id, checked) } }
            FormRow { label: qsTr("Resolution"); detail: modelData.resolution + " @ " + modelData.refresh + " Hz"
                PopUpButton { width: 230; model: modelData.modes; textRole: "label"
                    currentIndex: modelData.modes.findIndex(m => m.id === modelData.currentMode)
                    onActivated: i => Displays.setMode(modelData.id, modelData.modes[i].id) } }
            FormRow { label: qsTr("Scale"); separator: false
                SegmentedControl { width: 300; model: ["100%", "125%", "150%", "175%", "200%"]
                    currentIndex: Math.max(0, [1, 1.25, 1.5, 1.75, 2].indexOf(modelData.scale))
                    onActivated: i => Displays.setScale(modelData.id, [1, 1.25, 1.5, 1.75, 2][i]) } }
        }
    }
    FormSection {
        title: qsTr("Night Shift")
        footer: qsTr("Night Shift shifts the colours of your display to the warmer end of the spectrum after dark.")
        FormRow { label: qsTr("Night Shift"); separator: false
            Toggle { checked: KdeConfig.read("kwinrc", "NightColor", "Active", false)
                     onToggled: { KdeConfig.write("kwinrc", "NightColor", "Active", checked); KdeConfig.reconfigureKWin() } } }
    }
}
