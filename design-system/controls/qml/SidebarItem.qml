// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Templates as T
import Lumen.Design
import Lumen.Motion

T.ItemDelegate {
    id: control
    property string symbol: ""
    property color symbolColor: Theme.accent
    //! Render the symbol inside a tinted chip (System Settings style) —
    //! \c symbolColor then fills the chip and the glyph is drawn white.
    property bool chip: false
    property bool selected: false
    property string badge: ""
    implicitHeight: 28
    implicitWidth: 180
    leftPadding: 10; rightPadding: 10
    Spring { id: h; to: control.hovered && !control.selected ? 1 : 0; preset: Spring.Smooth }

    contentItem: Row {
        spacing: 7
        Rectangle {
            visible: control.chip && control.symbol.length > 0
            width: 22; height: 22; radius: Theme.metrics.controlRadius
            color: control.symbolColor
            anchors.verticalCenter: parent.verticalCenter
            Symbol { anchors.centerIn: parent; width: 14; height: 14; name: control.symbol; weight: Symbol.Fill; color: "white" }
        }
        Symbol {
            visible: !control.chip && control.symbol.length > 0
            name: control.symbol; width: 16; height: 16
            color: control.symbolColor
            anchors.verticalCenter: parent.verticalCenter
        }
        LText {
            text: control.text
            anchors.verticalCenter: parent.verticalCenter
            width: control.availableWidth - 30 - (control.badge.length ? 30 : 0)
        }
    }
    LText {
        visible: control.badge.length > 0
        anchors.right: parent.right; anchors.rightMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        text: control.badge; secondary: true; style: LText.Callout
    }
    background: Rectangle {
        radius: Theme.metrics.hoverRadius
        color: control.selected ? Theme.controlFillPressed
                                : Qt.rgba(Theme.controlFill.r, Theme.controlFill.g, Theme.controlFill.b, Theme.controlFill.a * h.value)
    }
}
