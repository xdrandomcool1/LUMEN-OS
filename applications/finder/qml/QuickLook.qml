// SPDX-License-Identifier: GPL-3.0-or-later
// Quick Look: Space previews the selected item (images, text, info).
import QtQuick
import QtQuick.Templates as T
import Lumen.Design
import Lumen.Motion
import Lumen.Glass
import Lumen.Controls

T.Popup {
    id: ql
    property var folder
    property var item: ({})
    property string text: ""
    function openFor(row) { item = folder.get(row); text = folder.previewText(row); open() }
    parent: T.Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - 80 : 800, 760)
    height: Math.min(parent ? parent.height - 60 : 600, 560)
    modal: false
    focus: true
    padding: 0
    enter: Transition { ParallelAnimation {
        NumberAnimation { property: "scale"; from: 0.9; to: 1; duration: Motion.normal; easing.type: Easing.Bezier; easing.bezierCurve: Motion.emphasized }
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Motion.fast } } }
    exit: Transition { NumberAnimation { property: "opacity"; to: 0; duration: Motion.instant } }
    background: GlassSurface { radius: Theme.metrics.windowRadius; material: Glass.Thin; backdrop: (ql.parent && ql.parent.Window.window && ql.parent.Window.window.lumenBackdrop) || null }
    contentItem: Item {
        Keys.onSpacePressed: ql.close()
        Keys.onEscapePressed: ql.close()
        LText { id: title; x: 16; y: 12; width: parent.width - 32; text: ql.item.name || ""; style: LText.Headline; horizontalAlignment: Text.AlignHCenter }
        Item {
            anchors.fill: parent; anchors.topMargin: 40; anchors.margins: 14
            Image { anchors.fill: parent; visible: ql.item.isImage === true; source: ql.item.isImage ? ql.item.url : ""; fillMode: Image.PreserveAspectFit; asynchronous: true; sourceSize: Qt.size(width * 2, height * 2) }
            Flickable {
                anchors.fill: parent; visible: ql.text.length > 0; clip: true; contentHeight: pre.implicitHeight
                Text { id: pre; width: parent.width; text: ql.text; font.family: Theme.monoFamily; font.pixelSize: 12; color: Theme.label; wrapMode: Text.WrapAnywhere; textFormat: Text.PlainText }
            }
            Column {
                anchors.centerIn: parent; spacing: 10
                visible: ql.item.isImage !== true && ql.text.length === 0
                FileIcon { anchors.horizontalCenter: parent.horizontalCenter; size: 128; iconName: ql.item.iconName || "" }
                LText { anchors.horizontalCenter: parent.horizontalCenter; text: (ql.item.kind || "") + "  ·  " + (ql.item.sizeText || ""); secondary: true }
                LText { anchors.horizontalCenter: parent.horizontalCenter; text: qsTr("Modified %1").arg(ql.item.modifiedText || ""); secondary: true; style: LText.Callout }
            }
        }
    }
}
