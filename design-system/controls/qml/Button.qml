// SPDX-License-Identifier: GPL-3.0-or-later
// Push button. Kinds: Bordered (default), Prominent (accent), Plain,
// Glass (Liquid Glass capsule), Destructive.
import QtQuick
import QtQuick.Templates as T
import Lumen.Design
import Lumen.Motion
import Lumen.Glass

T.Button {
    id: control
    enum Kind { Bordered, Prominent, Plain, Glass, Destructive }
    enum Size { Small, Regular, Large }

    property int kind: Button.Bordered
    property int size: Button.Regular
    property string symbol: ""
    property int symbolWeight: Symbol.Regular
    property BackdropBlur backdrop: (Window.window && Window.window.lumenBackdrop) ? Window.window.lumenBackdrop : null

    readonly property real _h: kind === Button.Glass ? [28, 34, 44][size] : [20, 24, 30][size]
    readonly property color _fg: {
        if (!enabled) return Theme.tertiaryLabel
        if (kind === Button.Prominent || kind === Button.Destructive) return Theme.textOnAccent
        if (kind === Button.Plain) return hovered ? Theme.label : Theme.secondaryLabel
        return Theme.label
    }

    implicitHeight: _h
    implicitWidth: Math.max(_h, implicitContentWidth + leftPadding + rightPadding)
    leftPadding: text.length ? (kind === Button.Glass ? 16 : 10) : 0
    rightPadding: leftPadding
    font.family: Theme.fontFamily
    font.pixelSize: size === Button.Large ? 14 : (size === Button.Small ? 11 : 13)
    font.weight: kind === Button.Prominent ? Font.DemiBold : Font.Medium
    focusPolicy: Qt.TabFocus

    Spring { id: press; to: control.down ? 1 : 0; preset: Spring.Snappy }
    Spring { id: hover; to: control.hovered ? 1 : 0; preset: Spring.Smooth }
    scale: 1 - 0.03 * press.value

    contentItem: Row {
        spacing: 5
        Symbol {
            visible: control.symbol.length > 0
            name: control.symbol
            weight: control.symbolWeight
            color: control._fg
            width: control.font.pixelSize + 3; height: width
            anchors.verticalCenter: parent.verticalCenter
        }
        Text {
            visible: control.text.length > 0
            text: control.text
            font: control.font
            color: control._fg
            anchors.verticalCenter: parent.verticalCenter
            renderType: Text.QtRendering
        }
    }

    background: Item {
        implicitHeight: control._h
        Rectangle {
            id: plate
            anchors.fill: parent
            visible: control.kind !== Button.Glass
            radius: control.kind === Button.Bordered && control.size !== Button.Large ? Theme.metrics.controlRadius : height / 2
            color: {
                switch (control.kind) {
                case Button.Prominent: return Qt.lighter(Theme.accent, 1 + 0.08 * hover.value - 0.12 * press.value)
                case Button.Destructive: return Qt.lighter(Theme.red, 1 + 0.08 * hover.value - 0.12 * press.value)
                case Button.Plain: return Qt.rgba(Theme.controlFill.r, Theme.controlFill.g, Theme.controlFill.b, Theme.controlFill.a * (hover.value + press.value))
                default: return press.value > 0.5 ? Theme.controlFillPressed : Theme.controlBackground
                }
            }
            border.width: control.kind === Button.Bordered ? 0.5 : 0
            border.color: Theme.controlBorder
            // Subtle top highlight on bordered/prominent buttons.
            Rectangle {
                anchors.fill: parent
                radius: parent.radius
                visible: control.kind === Button.Prominent || control.kind === Button.Destructive
                gradient: Gradient {
                    GradientStop { position: 0; color: Qt.rgba(1, 1, 1, 0.18) }
                    GradientStop { position: 1; color: Qt.rgba(1, 1, 1, 0) }
                }
            }
        }
        Rectangle { // drop shadow for bordered buttons in light mode
            z: -1
            visible: control.kind === Button.Bordered && !Theme.dark
            anchors.fill: plate; anchors.topMargin: 0.5; anchors.bottomMargin: -0.5
            radius: plate.radius
            color: Qt.rgba(0, 0, 0, 0.10)
        }
        GlassSurface {
            anchors.fill: parent
            visible: control.kind === Button.Glass
            radius: height / 2
            material: Glass.Floating
            backdrop: control.backdrop
            pressed: press.value
            shadowScale: 0.6
            shadowOffsetY: 3
        }
        Rectangle { // focus ring
            anchors.fill: parent; anchors.margins: -3
            radius: plate.radius + 3
            color: "transparent"
            border.width: 3
            border.color: Theme.focusRing
            visible: control.visualFocus
        }
    }
}
