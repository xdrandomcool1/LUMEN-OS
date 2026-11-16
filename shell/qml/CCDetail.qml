// SPDX-License-Identifier: GPL-3.0-or-later
// Detail page inside Control Center (Wi-Fi list, Bluetooth devices, …).
import QtQuick
import Lumen.Design
import Lumen.Controls
import Lumen.Shell

Column {
    id: d
    property string title
    property bool showToggle: true
    property bool toggleChecked: false
    property string settingsPage: ""
    default property alias body: bodyItem.data
    signal back()
    signal toggleClicked()
    spacing: 8

    Item {
        width: d.width; height: 28
        Row {
            spacing: 6; anchors.verticalCenter: parent.verticalCenter
            Symbol { width: 14; height: 14; name: "caret-left"; weight: Symbol.Bold; color: Theme.secondaryLabel; anchors.verticalCenter: parent.verticalCenter
                     MouseArea { anchors.fill: parent; anchors.margins: -8; onClicked: d.back() } }
            LText { text: d.title; style: LText.Title3 }
        }
        Toggle { visible: d.showToggle; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; checked: d.toggleChecked; onClicked: d.toggleClicked() }
    }
    Separator { width: d.width }
    Item { id: bodyItem; width: d.width; height: childrenRect.height }
    Separator { width: d.width }
    LText {
        text: qsTr("%1 Settings…").arg(d.title)
        leftPadding: 4
        MouseArea { anchors.fill: parent; onClicked: ShellController.openSettings(d.settingsPage) }
    }
}
