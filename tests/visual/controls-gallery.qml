import QtQuick
import Lumen.Design
import Lumen.Glass
import Lumen.Controls

Rectangle {
    id: root
    width: 900; height: 560
    color: Theme.windowBackground
    property string wallpaper: "file:///src/build/wp-preview/lumen-dusk.jpg"
    Image { id: wall; x: 470; y: 0; width: 430; height: 560; source: root.wallpaper; fillMode: Image.PreserveAspectCrop }
    BackdropBlur { id: blur; source: wall }

    Column {
        x: 30; y: 30; spacing: 16
        LText { text: "Controls"; style: LText.LargeTitle }
        Row { spacing: 10
            Button { text: "Cancel" }
            Button { text: "Continue"; kind: Button.Prominent }
            Button { text: "Delete"; kind: Button.Destructive }
            Button { text: "Plain"; kind: Button.Plain; symbol: "star" }
        }
        Row { spacing: 18
            Toggle { checked: true }
            Toggle { checked: false }
            CheckBox { checked: true; text: "Show hidden files" }
        }
        Slider { width: 300; value: 0.6 }
        SegmentedControl { width: 260; model: ["Icons", "List", "Columns"]; currentIndex: 1 }
        SearchField { width: 260; text: "" }
        TextField { width: 260; placeholderText: "Computer name" }
        ProgressBar { width: 300; value: 0.42 }
        Row { spacing: 12; Spinner {} PopUpButton { model: ["Automatic", "Light", "Dark"] } }
        FormSection { width: 400
            FormRow { label: "Wi-Fi"; symbol: "wifi-high"; Toggle { checked: true; small: true } }
            FormRow { label: "Bluetooth"; symbol: "bluetooth"; symbolTint: Theme.blue; detail: "On"; separator: false }
        }
    }
    Column {
        x: 500; y: 40; spacing: 20
        ToolbarGroup { ToolbarButton { symbol: "caret-left" } ToolbarButton { symbol: "caret-right" } }
        Button { kind: Button.Glass; text: "Glass button"; backdrop: blur }
        TrafficLights {}
        GlassSurface {
            width: 220; height: 140; radius: 12; material: Glass.Thin; backdrop: blur
            Column { anchors.fill: parent; anchors.margins: 5
                MenuItem { width: 210; text: "New Folder"; shortcutText: "⇧⌘N" }
                MenuItem { width: 210; text: "Get Info"; highlighted: true; shortcutText: "⌘I" }
                MenuSeparator { width: 210 }
                MenuItem { width: 210; text: "Move to Trash"; shortcutText: "⌘⌫" }
            }
        }
    }
}
