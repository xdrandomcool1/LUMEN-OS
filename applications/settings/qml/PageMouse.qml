import QtQuick
import Lumen.Design
import Lumen.Controls
import Lumen.Settings

SettingsPage {
    heading: qsTr("Mouse & Trackpad"); symbol: "mouse-simple"; tint: Theme.gray
    summary: InputDevices.devices.length === 0 ? qsTr("No pointing devices reported by the compositor.") : ""
    Repeater {
        model: InputDevices.devices
        FormSection {
            required property var modelData
            title: modelData.name + (modelData.touchpad ? qsTr(" — Trackpad") : "")
            FormRow { label: qsTr("Tracking speed")
                Slider { width: 200; from: -1; to: 1; value: modelData.pointerAcceleration; onMoved: InputDevices.set(modelData.sysName, "pointerAcceleration", value) } }
            FormRow { label: qsTr("Natural scrolling"); detail: qsTr("Content tracks finger movement")
                Toggle { checked: modelData.naturalScroll; onToggled: InputDevices.set(modelData.sysName, "naturalScroll", checked) } }
            FormRow { visible: modelData.touchpad === true; label: qsTr("Tap to click")
                Toggle { checked: modelData.tapToClick === true; onToggled: InputDevices.set(modelData.sysName, "tapToClick", checked) } }
            FormRow { label: qsTr("Primary button on the right"); separator: false
                Toggle { checked: modelData.leftHanded === true; onToggled: InputDevices.set(modelData.sysName, "leftHanded", checked) } }
        }
    }
}
