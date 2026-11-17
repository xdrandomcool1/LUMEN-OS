// SPDX-License-Identifier: GPL-3.0-or-later
// Wallpaper image with a cross-fade whenever the source changes (dynamic
// wallpapers, user selection). Used by the desktop and, invisibly, as the
// in-process backdrop for glass surfaces.
import QtQuick
import Lumen.Motion
import Lumen.Shell
import Lumen.System

Item {
    id: root
    property url source: WallpaperEngine.source
    property bool animate: true
    property size screenSize: Qt.size(width, height)

    property Image current: imgA
    Image {
        id: imgA
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop
        asynchronous: true
        cache: true
        sourceSize: root.screenSize
        smooth: true
    }
    Image {
        id: imgB
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop
        asynchronous: true
        cache: true
        sourceSize: root.screenSize
        smooth: true
        opacity: 0
    }
    function swap() {
        const next = current === imgA ? imgB : imgA
        next.source = root.source
    }
    Connections {
        target: imgA
        function onStatusChanged() { if (imgA.status === Image.Ready && root.current !== imgA && imgA.source == root.source) root.fadeTo(imgA) }
    }
    Connections {
        target: imgB
        function onStatusChanged() { if (imgB.status === Image.Ready && root.current !== imgB && imgB.source == root.source) root.fadeTo(imgB) }
    }
    function fadeTo(img) {
        const old = current
        current = img
        img.z = 1; old.z = 0
        if (!animate || Motion.reduceMotion) { img.opacity = 1; old.opacity = 0; return }
        fade.target = img
        fade.restart()
    }
    NumberAnimation { id: fade; property: "opacity"; from: 0; to: 1; duration: 900; easing.type: Easing.InOutQuad
        onFinished: (root.current === imgA ? imgB : imgA).opacity = 0 }
    onSourceChanged: swap()
    Component.onCompleted: { imgA.source = root.source; imgA.opacity = 1 }
}
