import QtQuick
import Lumen.Design
import Lumen.Controls
import Lumen.System

SettingsPage {
    FormSection {
        title: qsTr("Dock")
        FormRow { label: qsTr("Size"); Slider { width: 200; from: 32; to: 96; stepSize: 2; value: ShellSettings.dockIconSize; onMoved: ShellSettings.dockIconSize = value } }
        FormRow { label: qsTr("Magnification"); Toggle { checked: ShellSettings.dockMagnification; onToggled: ShellSettings.dockMagnification = checked } }
        FormRow { label: qsTr("Magnified size"); enabled: ShellSettings.dockMagnification
                  Slider { width: 200; from: ShellSettings.dockIconSize; to: 140; value: ShellSettings.dockMagnifiedSize; onMoved: ShellSettings.dockMagnifiedSize = value } }
        FormRow { label: qsTr("Automatically hide and show the Dock"); separator: false; Toggle { checked: ShellSettings.dockAutoHide; onToggled: ShellSettings.dockAutoHide = checked } }
    }
    FormSection {
        title: qsTr("Menu Bar")
        FormRow { label: qsTr("Show menu bar background"); Toggle { checked: ShellSettings.menuBarBackground; onToggled: ShellSettings.menuBarBackground = checked } }
        FormRow { label: qsTr("Show battery percentage"); Toggle { checked: ShellSettings.showBatteryPercentage; onToggled: ShellSettings.showBatteryPercentage = checked } }
        FormRow { label: qsTr("24-hour clock"); separator: false; Toggle { checked: ShellSettings.clock24h; onToggled: ShellSettings.clock24h = checked } }
    }
    FormSection {
        title: qsTr("Desktop Widgets")
        Repeater {
            model: [["clock", qsTr("Clock")], ["calendar", qsTr("Calendar")], ["weather", qsTr("Weather")], ["system", qsTr("System Monitor")]]
            FormRow {
                required property var modelData
                label: modelData[1]
                Toggle {
                    checked: ShellSettings.desktopWidgets.indexOf(modelData[0]) >= 0
                    onToggled: {
                        let l = ShellSettings.desktopWidgets.slice()
                        if (checked) l.push(modelData[0]); else l = l.filter(w => w !== modelData[0])
                        ShellSettings.desktopWidgets = l
                    }
                }
            }
        }
    }
    FormSection {
        title: qsTr("Weather Location")
        footer: qsTr("Weather data comes from Open-Meteo. Nothing is fetched until a location is set.")
        FormRow { label: qsTr("City"); TextField { width: 200; text: ShellSettings.weatherCity; onEditingFinished: ShellSettings.weatherCity = text } }
        FormRow { label: qsTr("Latitude"); TextField {
            width: 120; text: ShellSettings.weatherLatitude
            validator: DoubleValidator { bottom: -90; top: 90 }
            onEditingFinished: ShellSettings.weatherLatitude = Number(text)
        } }
        FormRow { label: qsTr("Longitude"); separator: false; TextField {
            width: 120; text: ShellSettings.weatherLongitude
            validator: DoubleValidator { bottom: -180; top: 180 }
            onEditingFinished: ShellSettings.weatherLongitude = Number(text)
        } }
    }
}
