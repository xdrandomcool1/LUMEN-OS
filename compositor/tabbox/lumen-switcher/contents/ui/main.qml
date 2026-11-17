// SPDX-License-Identifier: GPL-3.0-or-later
// Application switcher (Super+Tab). KWin runs this layout with
// [TabBox] ApplicationsMode=1, so each entry is one application.
import QtQuick
import QtQuick.Window
import org.kde.kirigami as Kirigami
import org.kde.kwin as KWin

KWin.TabBoxSwitcher {
    id: tabBox
    currentIndex: icons.currentIndex

    Window {
        id: dialog
        visible: tabBox.visible
        flags: Qt.BypassWindowManagerHint | Qt.FramelessWindowHint
        color: "transparent"
        width: Math.min(icons.contentWidth + 40, tabBox.screenGeometry.width - 80)
        height: 170
        x: tabBox.screenGeometry.x + (tabBox.screenGeometry.width - width) / 2
        y: tabBox.screenGeometry.y + (tabBox.screenGeometry.height - height) / 2

        Rectangle {
            anchors.fill: parent
            radius: 30
            color: Qt.rgba(0.12, 0.12, 0.13, 0.72)
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, 0.16)
        }
        ListView {
            id: icons
            anchors.fill: parent
            anchors.margins: 20
            anchors.bottomMargin: 44
            orientation: ListView.Horizontal
            spacing: 14
            model: tabBox.model
            currentIndex: tabBox.currentIndex
            highlightMoveDuration: 120
            highlight: Rectangle { radius: 20; color: Qt.rgba(1, 1, 1, 0.18) }
            delegate: Item {
                required property var model
                required property int index
                width: 108; height: 108
                Kirigami.Icon { anchors.centerIn: parent; width: 88; height: 88; source: model.icon }
                TapHandler { onTapped: { icons.currentIndex = index; tabBox.model.activate(index) } }
            }
        }
        Text {
            anchors.bottom: parent.bottom; anchors.bottomMargin: 14
            anchors.horizontalCenter: parent.horizontalCenter
            text: icons.currentItem ? icons.currentItem.model.caption : ""
            color: "white"; font.family: "Inter Variable"; font.pixelSize: 14; font.weight: Font.DemiBold
            elide: Text.ElideMiddle; width: parent.width - 40; horizontalAlignment: Text.AlignHCenter
        }
    }
}
