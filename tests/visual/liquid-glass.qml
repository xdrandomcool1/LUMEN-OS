// Visual QA scene for Liquid Glass refraction: glass shapes over a backdrop
// with fine detail (text, stripes, a grid) so lensing at the rim, dispersion
// and the clear centre are easy to judge.
//
//   lumen-qmlgrab tests/visual/liquid-glass.qml out.png --at 1500
//   (QML_IMPORT_PATH=build/qml; optional --eval "root.dark = true")
import QtQuick
import Lumen.Design
import Lumen.Glass

Item {
    id: root
    width: 1440; height: 900
    property string wallpaper: "file:///src/build/wallpapers/aurora.jpg"
    property bool dark: false
    Component.onCompleted: Appearance.mode = dark ? Appearance.Dark : Appearance.Light
    onDarkChanged: Appearance.mode = dark ? Appearance.Dark : Appearance.Light

    Item {
        id: scene
        anchors.fill: parent
        Image { anchors.fill: parent; source: root.wallpaper; fillMode: Image.PreserveAspectCrop; smooth: true }
        // Diagonal stripes and a grid: straight lines reveal bending.
        Repeater {
            model: 24
            Rectangle {
                required property int index
                x: index * 64 - 200; y: -100; width: 10; height: 1200
                rotation: 24
                color: index % 3 === 0 ? "#ff4d6d" : index % 3 === 1 ? "#ffd166" : "#06d6a0"
                opacity: 0.75
            }
        }
        Repeater {
            model: 16
            Rectangle { required property int index; x: 0; y: index * 60; width: root.width; height: 1; color: "#80ffffff" }
        }
        Column {
            x: 40; y: 30; spacing: 6
            Repeater {
                model: 12
                Text {
                    required property int index
                    text: "LUMEN OS — Liquid Glass bends light at the edge, the centre stays clear. " + (index + 1)
                    font.pixelSize: 26; font.weight: Font.DemiBold
                    color: index % 2 ? "#1b1b1f" : "white"
                }
            }
        }
    }
    BackdropBlur { id: blur; source: scene }

    // Dock-like capsule
    GlassSurface {
        x: 400; y: 760; width: 640; height: 84; radius: 32
        backdrop: blur; material: Glass.UltraThin
    }
    // Round button (lens)
    GlassSurface {
        x: 120; y: 520; width: 180; height: 180; radius: 90; smoothness: 2
        backdrop: blur; material: Glass.Floating
    }
    // Panels in each material
    Repeater {
        model: [ [Glass.UltraThin, "Clear"], [Glass.Thin, "Thin"], [Glass.Regular, "Regular"], [Glass.Frosted, "Frosted"] ]
        GlassSurface {
            required property var modelData
            required property int index
            x: 380 + index * 260; y: 470; width: 230; height: 230; radius: 34
            backdrop: blur; material: modelData[0]
            LText { anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom; anchors.bottomMargin: 16
                    text: modelData[1]; style: LText.Headline; color: root.dark ? "white" : "#111" }
        }
    }
    // Wide toolbar
    GlassSurface {
        x: 820; y: 60; width: 520; height: 64; radius: 32
        backdrop: blur; material: Glass.Regular
    }
}
