import QtQuick
import Lumen.Design
import Lumen.Controls
import Lumen.System
import Lumen.Settings

SettingsPage {
    id: page
    heading: qsTr("Keyboard"); symbol: "keyboard"; tint: Theme.gray
    property var layoutCodes: String(KdeConfig.read("kxkbrc", "Layout", "LayoutList", "us")).split(",").filter(s => s.length)
    function saveLayouts(l) {
        layoutCodes = l
        KdeConfig.write("kxkbrc", "Layout", "LayoutList", l.join(","))
        KdeConfig.write("kxkbrc", "Layout", "Use", true)
        KdeConfig.reloadKeyboard()
    }
    FormSection {
        title: qsTr("Key repeat")
        FormRow { label: qsTr("Delay until repeat")
            Slider { width: 200; from: 1000; to: 150; value: KdeConfig.read("kcminputrc", "Keyboard", "RepeatDelay", 600)
                     onMoved: KdeConfig.write("kcminputrc", "Keyboard", "RepeatDelay", Math.round(value)) } }
        FormRow { label: qsTr("Key repeat rate"); separator: false
            Slider { width: 200; from: 5; to: 60; value: KdeConfig.read("kcminputrc", "Keyboard", "RepeatRate", 25)
                     onMoved: KdeConfig.write("kcminputrc", "Keyboard", "RepeatRate", Math.round(value)) } }
    }
    FormSection {
        title: qsTr("Input sources")
        Repeater {
            model: page.layoutCodes
            FormRow { required property string modelData; required property int index
                label: (XkbLayouts.all.find(l => l.code === modelData) || { name: modelData }).name
                detail: KeyboardLayout.current === index ? qsTr("Active") : ""
                Button { size: Button.Small; text: qsTr("Remove"); enabled: page.layoutCodes.length > 1
                         onClicked: page.saveLayouts(page.layoutCodes.filter(c => c !== modelData)) } }
        }
        FormRow { label: qsTr("Add input source"); separator: false
            PopUpButton { width: 240; model: XkbLayouts.all; textRole: "name"; displayText: qsTr("Choose…")
                          onActivated: i => { const c = XkbLayouts.all[i].code; if (page.layoutCodes.indexOf(c) < 0) page.saveLayouts(page.layoutCodes.concat([c])) } } }
    }
    FormSection {
        title: qsTr("Shortcuts")
        Repeater {
            model: [[qsTr("Spotlight"), "Super+Space"], [qsTr("Control Center"), "Super+C"], [qsTr("Notification Center"), "Super+N"],
                    [qsTr("Application switcher"), "Super+Tab"], [qsTr("Mission Control"), "Super+↑ / three-finger swipe up"],
                    [qsTr("Minimize window"), "Super+M"], [qsTr("Close window"), "Super+W"], [qsTr("Show desktop"), "Super+D"]]
            FormRow { required property var modelData; label: modelData[0]; LText { text: modelData[1]; secondary: true } }
        }
    }
}
