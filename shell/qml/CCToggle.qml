// SPDX-License-Identifier: GPL-3.0-or-later
// Circular toggle + label (Wi-Fi, Bluetooth, Focus…).
import QtQuick
import Lumen.Design
import Lumen.Motion

Item {
    id: t
    property string symbol
    property string title
    property string subtitle
    property bool checked: false
    property bool showChevron: false
    property real circle: 30
    signal toggled()
    signal detailsRequested()
    implicitHeight: circle
    implicitWidth: 200

    Spring { id: on; to: t.checked ? 1 : 0; preset: Spring.Snappy }
    Spring { id: press; to: circleArea.pressed ? 1 : 0; preset: Spring.Snappy }

    Rectangle {
        id: disc
        width: t.circle; height: t.circle; radius: width / 2
        scale: 1 - 0.08 * press.value
        color: on.value > 0.5 ? Theme.accent : (Theme.dark ? Qt.rgba(1, 1, 1, 0.14) : Qt.rgba(0, 0, 0, 0.08))
        Behavior on color { ColorAnimation { duration: Motion.fast } }
        Symbol {
            anchors.centerIn: parent
            width: t.circle * 0.53; height: width
            name: t.symbol; weight: Symbol.Fill
            color: on.value > 0.5 ? "white" : Theme.label
        }
        MouseArea { id: circleArea; anchors.fill: parent; onClicked: t.toggled() }
    }
    Column {
        anchors.left: disc.right; anchors.leftMargin: 8
        anchors.right: parent.right
        anchors.verticalCenter: disc.verticalCenter
        LText { width: parent.width; text: t.title; style: LText.Callout; font.weight: Font.DemiBold }
        LText { width: parent.width; visible: t.subtitle.length > 0; text: t.subtitle; style: LText.Caption; secondary: true }
    }
    MouseArea {
        anchors.left: disc.right; anchors.right: parent.right; height: parent.height
        enabled: t.showChevron
        onClicked: t.detailsRequested()
    }
}
