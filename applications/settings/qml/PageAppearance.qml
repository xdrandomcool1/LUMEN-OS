import QtQuick
import Lumen.Design
import Lumen.Controls

SettingsPage {
    FormSection {
        title: qsTr("Appearance")
        Item {
            width: parent.width; height: 132
            Row {
                anchors.centerIn: parent; spacing: 26
                Repeater {
                    model: [[Appearance.Light, qsTr("Light"), "#F2F2F5", "#FFFFFF"], [Appearance.Dark, qsTr("Dark"), "#1C1C1E", "#3A3A3C"], [Appearance.Auto, qsTr("Auto"), "#F2F2F5", "#1C1C1E"]]
                    Column {
                        required property var modelData
                        spacing: 6
                        Rectangle {
                            width: 96; height: 64; radius: 10; clip: true
                            border.width: Appearance.mode === modelData[0] ? 3 : 1
                            border.color: Appearance.mode === modelData[0] ? Theme.accent : Theme.controlBorder
                            gradient: Gradient { orientation: Gradient.Horizontal
                                GradientStop { position: 0.5; color: modelData[2] } GradientStop { position: 0.5001; color: modelData[3] } }
                            Rectangle { x: 10; y: 10; width: 46; height: 30; radius: 5; color: modelData[0] === Appearance.Dark ? "#2C2C2E" : "white"; border.width: 0.5; border.color: "#33000000" }
                            MouseArea { anchors.fill: parent; onClicked: Appearance.mode = modelData[0] }
                        }
                        LText { anchors.horizontalCenter: parent.horizontalCenter; text: modelData[1]; style: LText.Callout }
                    }
                }
            }
        }
        FormRow {
            label: qsTr("Accent color")
            Row {
                spacing: 8
                Repeater {
                    model: Appearance.accentNames
                    Rectangle {
                        required property string modelData
                        width: 18; height: 18; radius: 9; color: Appearance.accentColor(modelData)
                        border.width: Appearance.accentName === modelData ? 2 : 0; border.color: Theme.label
                        MouseArea { anchors.fill: parent; onClicked: Appearance.accentName = modelData }
                    }
                }
            }
        }
        FormRow {
            label: qsTr("Liquid Glass"); detail: qsTr("How strongly glass bends and catches light"); separator: false
            Slider { width: 200; from: 0; to: 1.5; value: Appearance.glassIntensity; onMoved: Appearance.glassIntensity = value }
        }
    }
}
