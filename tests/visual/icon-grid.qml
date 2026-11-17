// Visual QA: the generated Lumen app icons (premium squircle set) rendered
// through Qt SVG at Dock size — dark and light wallpapers side by side.
import QtQuick
import Lumen.Design
import Lumen.Glass

Rectangle {
    id: root
    width: 1440; height: 900
    readonly property string dir: "file:///src/themes/icons/Lumen/scalable/apps/"
    readonly property var icons: [
        { n: "lumen-finder", t: "Finder" },
        { n: "lumen-settings", t: "Settings" },
        { n: "lumen-terminal", t: "Terminal" },
        { n: "lumen-store", t: "App Store" },
        { n: "lumen-activity-monitor", t: "Activity Monitor" },
        { n: "lumen-calculator", t: "Calculator" },
        { n: "lumen-calendar", t: "Calendar" },
        { n: "lumen-texteditor", t: "Text Editor" },
        { n: "lumen-imageviewer", t: "Image Viewer" },
        { n: "lumen-mediaplayer", t: "Media Player" },
        { n: "lumen-screenshot", t: "Screenshot" },
        { n: "lumen-logo", t: "LUMEN" }
    ]

    Rectangle {
        x: 0; width: parent.width / 2; height: parent.height
        gradient: Gradient {
            GradientStop { position: 0; color: "#2C1E5C" }
            GradientStop { position: 1; color: "#B4485B" }
        }
    }
    Rectangle {
        x: parent.width / 2; width: parent.width / 2; height: parent.height
        gradient: Gradient {
            GradientStop { position: 0; color: "#D8DEE8" }
            GradientStop { position: 1; color: "#8FA2C4" }
        }
    }

    Grid {
        anchors.centerIn: parent
        columns: 4
        spacing: 44
        Repeater {
            model: root.icons
            delegate: Column {
                required property var modelData
                spacing: 10
                Image {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 92; height: 92
                    source: root.dir + modelData.n + ".svg"
                    sourceSize: Qt.size(256, 256)
                    mipmap: true
                    smooth: true
                }
                LText {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 120
                    horizontalAlignment: Text.AlignHCenter
                    text: modelData.t
                    style: LText.Caption
                }
            }
        }
    }
}
