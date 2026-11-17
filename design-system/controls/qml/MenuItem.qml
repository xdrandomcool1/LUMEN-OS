// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Templates as T
import Lumen.Design

T.MenuItem {
    id: control
    property string symbol: ""
    property string shortcutText: action && action.shortcut ? action.shortcut.toString() : ""
    property bool destructive: false

    implicitWidth: leftPadding + contentRow.implicitWidth + rightPadding
    implicitHeight: Theme.metrics.menuItemHeight
    leftPadding: 22
    rightPadding: 12
    readonly property bool _hi: control.highlighted && control.enabled
    readonly property color _fg: !enabled ? Theme.tertiaryLabel : _hi ? "white" : (destructive ? Theme.red : Theme.label)

    contentItem: Row {
        id: contentRow
        spacing: 6
        Symbol {
            visible: control.symbol.length > 0
            name: control.symbol; width: 15; height: 15; color: control._fg
            anchors.verticalCenter: parent.verticalCenter
        }
        Text {
            text: control.text
            font.family: Theme.fontFamily; font.pixelSize: 13
            color: control._fg
            anchors.verticalCenter: parent.verticalCenter
            renderType: Text.QtRendering
        }
        Item { width: control.shortcutText.length || control.subMenu ? 24 : 0; height: 1 }
    }

    // Shortcut / submenu chevron pinned to the right edge.
    Text {
        anchors.right: parent.right; anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        visible: control.shortcutText.length > 0 && !control.subMenu
        text: control.shortcutText
        font.family: Theme.fontFamily; font.pixelSize: 13
        color: control._hi ? Qt.rgba(1, 1, 1, 0.8) : Theme.tertiaryLabel
        renderType: Text.QtRendering
    }
    arrow: Symbol {
        x: control.width - width - 8
        anchors.verticalCenter: parent.verticalCenter
        visible: control.subMenu
        name: "caret-right"; weight: Symbol.Bold; width: 11; height: 11; color: control._fg
    }
    indicator: Symbol {
        x: 6
        anchors.verticalCenter: parent.verticalCenter
        visible: control.checked
        name: "check"; weight: Symbol.Bold; width: 12; height: 12; color: control._fg
    }
    background: Item {
        implicitWidth: 160
        implicitHeight: Theme.metrics.menuItemHeight
        Rectangle {
            anchors.fill: parent
            radius: Theme.metrics.menuItemRadius
            color: control.destructive ? Theme.red : Theme.menuHighlight
            visible: control._hi
        }
    }
}
