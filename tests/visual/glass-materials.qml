// Visual QA scene: every glass material over a busy wallpaper.
import QtQuick
import Lumen.Design
import Lumen.Glass

Item {
    id: root
    width: 1440; height: 900
    property string wallpaper: "file:///src/build/wallpapers/lumen-dusk.jpg"

    Image { id: wall; anchors.fill: parent; source: root.wallpaper; fillMode: Image.PreserveAspectCrop; smooth: true }
    BackdropBlur { id: blur; source: wall }

    Repeater {
        model: [ [Glass.UltraThin, "Ultra-thin"], [Glass.Thin, "Thin"], [Glass.Regular, "Regular"], [Glass.Frosted, "Frosted"],
                 [Glass.Dark, "Dark"], [Glass.Light, "Light"], [Glass.Floating, "Floating"], [Glass.Active, "Active"] ]
        GlassSurface {
            required property var modelData
            required property int index
            x: 80 + (index % 4) * 330; y: 120 + Math.floor(index / 4) * 260
            width: 280; height: 180; radius: 26
            backdrop: blur; material: modelData[0]
            LText { anchors.centerIn: parent; text: modelData[1]; style: LText.Title2
                    color: modelData[0] === Glass.Light ? "#111" : (modelData[0] === Glass.Dark || Appearance.dark ? "white" : "#111") }
        }
    }
    // Dock-like capsule
    GlassSurface {
        anchors.horizontalCenter: parent.horizontalCenter; y: 900 - 8 - 76
        width: 640; height: 76; radius: 30; backdrop: blur; material: Glass.UltraThin
    }
}
