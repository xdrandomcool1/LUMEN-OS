// SPDX-License-Identifier: GPL-3.0-or-later
// Get Info inspector.
import QtQuick
import QtQuick.Templates as T
import Lumen.Design
import Lumen.Glass
import Lumen.Controls

T.Popup {
    id: ip
    property var folder
    property var item: ({})
    function openFor(row) { item = folder.get(row); open() }
    parent: T.Overlay.overlay
    x: parent ? parent.width - width - 20 : 0; y: 70
    width: 300
    height: col.implicitHeight + 32
    padding: 16
    focus: true
    background: GlassSurface { radius: Theme.metrics.windowRadius; material: Glass.Regular; backdrop: (ip.parent && ip.parent.Window.window && ip.parent.Window.window.lumenBackdrop) || null }
    contentItem: Column {
        id: col
        spacing: 10
        Row {
            spacing: 10
            FileIcon { size: 48; iconName: ip.item.iconName || ""; url: ip.item.url || ""; isImage: ip.item.isImage === true }
            Column {
                anchors.verticalCenter: parent.verticalCenter
                LText { text: ip.item.name || ""; style: LText.Headline; width: 200 }
                LText { text: ip.item.sizeText || ""; secondary: true; style: LText.Callout }
            }
        }
        Separator { width: col.width }
        Repeater {
            model: [[qsTr("Kind"), ip.item.kind], [qsTr("Where"), ip.item.path ? ip.item.path.substring(0, ip.item.path.lastIndexOf("/")) : ""],
                    [qsTr("Modified"), ip.item.modifiedText], [qsTr("Type"), ip.item.mimeType]]
            Row {
                required property var modelData
                spacing: 8
                LText { width: 70; text: modelData[0] + ":"; secondary: true; horizontalAlignment: Text.AlignRight; style: LText.Callout }
                LText { width: 190; text: modelData[1] || "—"; style: LText.Callout; wrapMode: Text.WrapAnywhere; elide: Text.ElideNone }
            }
        }
    }
}
