// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Templates as T
import Lumen.Design
import Lumen.Motion

T.ToolButton {
    id: control
    property string symbol: ""
    property string tooltip: ""
    implicitWidth: text.length ? contentItem.implicitWidth + 20 : 34
    implicitHeight: 30
    Spring { id: h; to: control.down ? 2 : (control.hovered ? 1 : 0); preset: Spring.Snappy }
    contentItem: Row {
        spacing: 5
        Symbol { name: control.symbol; width: 17; height: 17; color: control.enabled ? (control.checked ? Theme.accent : Theme.label) : Theme.tertiaryLabel; anchors.verticalCenter: parent.verticalCenter }
        LText { visible: control.text.length > 0; text: control.text; anchors.verticalCenter: parent.verticalCenter }
    }
    background: Rectangle {
        radius: height / 2
        anchors.fill: parent; anchors.margins: 1
        color: Qt.rgba(Theme.controlFill.r, Theme.controlFill.g, Theme.controlFill.b, Math.min(1, Theme.controlFill.a * h.value * 1.3))
    }
    ToolTip { visible: control.hovered && control.tooltip.length > 0; text: control.tooltip }
}
