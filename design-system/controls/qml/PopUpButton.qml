// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Templates as T
import Lumen.Design
import Lumen.Glass

T.ComboBox {
    id: control
    implicitWidth: Math.max(120, contentItem.implicitWidth + 40)
    implicitHeight: 24
    leftPadding: 9
    rightPadding: 26
    font.family: Theme.fontFamily
    font.pixelSize: 13

    delegate: MenuItem {
        required property var model
        required property int index
        width: ListView.view ? ListView.view.width : implicitWidth
        text: model[control.textRole] !== undefined ? model[control.textRole] : modelData
        checked: control.currentIndex === index
        highlighted: control.highlightedIndex === index
    }
    indicator: Rectangle {
        x: control.width - width - 3; y: (control.height - height) / 2
        width: 18; height: 18; radius: 5
        color: Theme.accent
        Symbol { anchors.centerIn: parent; width: 12; height: 12; name: "caret-up-down"; weight: Symbol.Bold; color: "white" }
    }
    contentItem: Text {
        text: control.displayText
        font: control.font
        color: Theme.label
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        renderType: Text.QtRendering
    }
    background: Rectangle {
        radius: 6
        color: Theme.controlBackground
        border.width: 0.5
        border.color: Theme.controlBorder
    }
    popup: T.Popup {
        y: control.height + 4
        width: Math.max(control.width, 160)
        implicitHeight: Math.min(contentItem.implicitHeight + 10, 400)
        padding: 5
        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.delegateModel
            currentIndex: control.highlightedIndex
        }
        background: GlassSurface {
            radius: 12; material: Glass.Thin
            backdrop: (Window.window && Window.window.lumenBackdrop) ? Window.window.lumenBackdrop : null
        }
    }
}
