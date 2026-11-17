// SPDX-License-Identifier: GPL-3.0-or-later
// Segmented control with a sliding selection pill.
import QtQuick
import Lumen.Design
import Lumen.Motion

Item {
    id: root
    property var model: []          // strings or {text, symbol}
    property int currentIndex: 0
    property bool symbolsOnly: false
    signal activated(int index)

    readonly property real segW: Math.max(28, (width - 4) / Math.max(1, model.length))
    implicitWidth: Math.max(model.length * (symbolsOnly ? 34 : 72), 60)
    implicitHeight: 24

    Rectangle {
        anchors.fill: parent
        radius: height / 2
        color: Theme.controlFill
    }
    Spring { id: pill; to: root.currentIndex * root.segW; preset: Spring.Snappy }
    Rectangle {
        x: 2 + pill.value; y: 2
        width: root.segW; height: root.height - 4
        radius: height / 2
        color: Theme.dark ? Qt.rgba(1, 1, 1, 0.22) : "white"
        Rectangle { z: -1; anchors.fill: parent; anchors.topMargin: 0.5; anchors.bottomMargin: -1; radius: parent.radius; color: Qt.rgba(0, 0, 0, Theme.dark ? 0 : 0.12) }
    }
    Row {
        x: 2; y: 2
        Repeater {
            model: root.model
            delegate: Item {
                required property var modelData
                required property int index
                width: root.segW; height: root.height - 4
                readonly property string label: typeof modelData === "string" ? modelData : (modelData.text || "")
                readonly property string sym: typeof modelData === "string" ? "" : (modelData.symbol || "")
                Row {
                    anchors.centerIn: parent
                    spacing: 4
                    Symbol { visible: sym.length > 0; name: sym; width: 14; height: 14; color: Theme.label; anchors.verticalCenter: parent.verticalCenter }
                    LText { visible: !root.symbolsOnly && label.length > 0; text: label; style: LText.Callout; font.weight: index === root.currentIndex ? Font.DemiBold : Font.Normal }
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: { root.currentIndex = index; root.activated(index) }
                }
            }
        }
    }
}
