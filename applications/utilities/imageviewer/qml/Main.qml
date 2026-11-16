// SPDX-License-Identifier: GPL-3.0-or-later
// Image Viewer: zoom, pan, rotate, browse the folder, slideshow.
import QtQuick
import QtQuick.Dialogs
import Qt.labs.folderlistmodel
import Lumen.Design
import Lumen.Motion
import Lumen.Controls

AppWindow {
    id: win
    property string openFile
    property url current: openFile
    title: current.toString().length ? decodeURIComponent(current.toString().split("/").pop()) : qsTr("Image Viewer")
    subtitle: img.status === Image.Ready ? img.sourceSize.width + " × " + img.sourceSize.height + "  ·  " + Math.round(zoom * 100) + "%" : ""
    width: 960; height: 680
    visible: true
    contentColor: Theme.dark ? "#141416" : "#F2F2F4"
    property real zoom: 1
    property int rotation: 0

    FolderListModel {
        id: folder
        folder: win.current.toString().length ? win.current.toString().substring(0, win.current.toString().lastIndexOf("/")) : ""
        nameFilters: ["*.png", "*.jpg", "*.jpeg", "*.webp", "*.gif", "*.bmp", "*.svg", "*.tif", "*.tiff", "*.avif"]
        showDirs: false
    }
    function step(d) {
        if (folder.count === 0) return
        let i = folder.indexOf(win.current); i = (i + d + folder.count) % folder.count
        win.current = folder.get(i, "fileUrl"); win.zoom = 1; win.rotation = 0
    }
    toolbarTrailing: [
        ToolbarGroup {
            ToolbarButton { symbol: "caret-left"; onClicked: win.step(-1) }
            ToolbarButton { symbol: "caret-right"; onClicked: win.step(1) }
        },
        ToolbarGroup {
            ToolbarButton { symbol: "magnifying-glass-minus"; onClicked: win.zoom = Math.max(0.1, win.zoom / 1.25) }
            ToolbarButton { symbol: "magnifying-glass-plus"; onClicked: win.zoom = Math.min(16, win.zoom * 1.25) }
            ToolbarButton { symbol: "arrow-clockwise"; tooltip: qsTr("Rotate"); onClicked: win.rotation = (win.rotation + 90) % 360 }
            ToolbarButton { symbol: "play"; tooltip: qsTr("Slideshow"); checked: slide.running; onClicked: slide.running = !slide.running }
            ToolbarButton { symbol: "folder-open"; onClicked: dlg.open() }
        }
    ]
    FileDialog { id: dlg; nameFilters: [qsTr("Images (*.png *.jpg *.jpeg *.webp *.gif *.svg *.bmp)")]; onAccepted: { win.current = selectedFile; win.zoom = 1 } }
    Timer { id: slide; interval: 3000; repeat: true; onTriggered: win.step(1) }
    Spring { id: zs; to: win.zoom; preset: Spring.Smooth }
    Spring { id: rs; to: win.rotation; preset: Spring.Smooth }

    Flickable {
        id: view
        x: 0; y: win.toolbarHeight
        width: win.contentArea.width; height: win.contentArea.height - y
        contentWidth: Math.max(width, img.paintedWidth * zs.value); contentHeight: Math.max(height, img.paintedHeight * zs.value)
        clip: true
        Image {
            id: img
            width: view.width; height: view.height
            anchors.centerIn: parent
            source: win.current
            fillMode: Image.PreserveAspectFit
            asynchronous: true
            autoTransform: true
            mipmap: true
            scale: zs.value
            rotation: rs.value
        }
        WheelHandler { acceptedModifiers: Qt.ControlModifier; onWheel: e => win.zoom = Math.max(0.1, Math.min(16, win.zoom * (e.angleDelta.y > 0 ? 1.1 : 0.9))) }
        PinchHandler { target: null; onScaleChanged: delta => win.zoom = Math.max(0.1, Math.min(16, win.zoom * delta)) }
    }
    Column {
        anchors.centerIn: parent; visible: !win.current.toString().length; spacing: 10
        Symbol { anchors.horizontalCenter: parent.horizontalCenter; width: 48; height: 48; name: "image"; color: Theme.tertiaryLabel }
        Button { text: qsTr("Open Image…"); onClicked: dlg.open() }
    }
    DropArea { anchors.fill: parent; onDropped: d => { if (d.hasUrls) win.current = d.urls[0] } }
    Shortcut { sequence: "Right"; onActivated: win.step(1) }
    Shortcut { sequence: "Left"; onActivated: win.step(-1) }
    Shortcut { sequence: "Ctrl+0"; onActivated: win.zoom = 1 }
    Shortcut { sequence: "Ctrl+R"; onActivated: win.rotation = (win.rotation + 90) % 360 }
}
