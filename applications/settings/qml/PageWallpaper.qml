import QtQuick
import Lumen.Design
import Lumen.Controls
import Lumen.System

SettingsPage {
    FormSection {
        title: qsTr("Dynamic")
        footer: qsTr("Dynamic wallpapers change with the time of day.")
        Flow {
            width: parent.width; padding: 12; spacing: 12
            Repeater {
                model: WallpaperEngine.dynamicSets
                Column {
                    required property string modelData
                    spacing: 4
                    Rectangle {
                        width: 128; height: 80; radius: 8; clip: true
                        border.width: ShellSettings.dynamicWallpaper === modelData ? 3 : 0; border.color: Theme.accent
                        Image { anchors.fill: parent; anchors.margins: parent.border.width; fillMode: Image.PreserveAspectCrop; asynchronous: true
                                sourceSize: Qt.size(256, 160); source: "file://" + WallpaperEngine.directory + "/" + WallpaperEngine.dynamicImageAt(modelData, new Date().getHours() * 60) + ".jpg" }
                        MouseArea { anchors.fill: parent; onClicked: WallpaperEngine.setDynamic(modelData) }
                    }
                    LText { text: modelData.charAt(0).toUpperCase() + modelData.slice(1); style: LText.Caption }
                }
            }
        }
    }
    FormSection {
        title: qsTr("Pictures")
        Flow {
            width: parent.width; padding: 12; spacing: 12
            Repeater {
                model: WallpaperEngine.wallpapers
                Column {
                    required property var modelData
                    spacing: 4
                    Rectangle {
                        width: 128; height: 80; radius: 8; clip: true
                        readonly property bool sel: !ShellSettings.dynamicWallpaper.length && ShellSettings.wallpaper === modelData.name
                        border.width: sel ? 3 : 0; border.color: Theme.accent
                        Image { anchors.fill: parent; anchors.margins: parent.border.width; source: modelData.source; fillMode: Image.PreserveAspectCrop; asynchronous: true; sourceSize: Qt.size(256, 160) }
                        MouseArea { anchors.fill: parent; onClicked: WallpaperEngine.setStatic(modelData.name) }
                    }
                    LText { width: 128; text: modelData.title; style: LText.Caption }
                }
            }
        }
    }
}
