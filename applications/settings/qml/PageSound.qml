import QtQuick
import Lumen.Design
import Lumen.Controls
import Lumen.System

SettingsPage {
    heading: qsTr("Sound"); symbol: "speaker-high"; tint: Theme.pink
    summary: Audio.available ? "" : qsTr("No audio output devices were found.")
    FormSection {
        title: qsTr("Output volume")
        FormRow { label: qsTr("Volume"); Row { spacing: 8
            Symbol { width: 14; height: 14; name: "speaker-simple-low"; color: Theme.secondaryLabel; anchors.verticalCenter: parent.verticalCenter }
            Slider { width: 220; value: Audio.volume; enabled: Audio.available; onMoved: Audio.volume = value }
            Symbol { width: 14; height: 14; name: "speaker-simple-high"; color: Theme.secondaryLabel; anchors.verticalCenter: parent.verticalCenter } } }
        FormRow { label: qsTr("Mute"); separator: false; Toggle { checked: Audio.muted; enabled: Audio.available; onToggled: Audio.muted = checked } }
    }
    FormSection {
        title: qsTr("Output device")
        visible: Audio.outputs.length > 0
        Repeater {
            model: Audio.outputs
            FormRow { required property var modelData; label: modelData.description; clickable: true; onClicked: Audio.setDefaultOutput(modelData.name)
                Symbol { visible: modelData.active; width: 14; height: 14; name: "check"; weight: Symbol.Bold; color: Theme.accent } }
        }
    }
}
