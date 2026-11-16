// SPDX-License-Identifier: GPL-3.0-or-later
// A notification (banner or Notification Center entry).
import QtQuick
import Lumen.Design
import Lumen.Motion
import Lumen.Glass
import Lumen.Controls
import Lumen.Shell

GlassSurface {
    id: card
    property var notifications
    property int row: -1
    property string appName
    property string appIcon
    property string summary
    property string body
    property var image
    property string iconName
    property date created
    property var actionNames: []
    property var actionLabels: []
    property bool hasDefaultAction: false
    signal dismissed()

    width: 352
    height: Math.max(66, col.implicitHeight + 24)
    radius: 22
    material: Glass.Regular
    shadowScale: 0.8
    shadowOffsetY: 6

    function idx() { return notifications.index(row, 0) }

    // Swipe right to dismiss.
    property real dragX: 0
    Spring { id: settle; to: 0; preset: Spring.Snappy; onValueChanged: if (!drag.active) card.dragX = value }
    transform: Translate { x: card.dragX }
    DragHandler {
        id: drag
        target: null
        xAxis.enabled: true; yAxis.enabled: false
        onTranslationChanged: card.dragX = Math.max(-20, translation.x)
        onActiveChanged: if (!active) {
            if (card.dragX > 90) { card.dismissed(); card.notifications.close(card.idx()) }
            else { settle.snap(card.dragX); settle.to = 0 }
        }
    }
    TapHandler {
        onTapped: {
            if (card.hasDefaultAction) card.notifications.invokeDefaultAction(card.idx(), 0)
            else ShellController.ShowPanel("notifications")
        }
    }
    HoverHandler { id: hov }

    AppIcon {
        x: 14; y: 14
        size: 34
        source: card.appIcon.length ? card.appIcon : "preferences-desktop-notification"
    }
    Column {
        id: col
        x: 58; y: 12
        width: card.width - 58 - 14
        spacing: 1
        Item {
            width: parent.width; height: 17
            LText { text: card.summary; style: LText.Headline; width: parent.width - 60 }
            LText { anchors.right: parent.right; text: Qt.formatTime(card.created, "HH:mm"); style: LText.Caption; secondary: true }
        }
        LText { width: parent.width; text: card.body; style: LText.Callout; secondary: true; wrapMode: Text.Wrap; maximumLineCount: 3; textFormat: Text.StyledText; visible: text.length > 0 }
        Row {
            visible: card.actionNames && card.actionNames.length > 0
            spacing: 6; topPadding: 6
            Repeater {
                model: card.actionNames ? card.actionNames.length : 0
                Button {
                    required property int index
                    text: card.actionLabels[index]
                    size: Button.Small
                    onClicked: card.notifications.invokeAction(card.idx(), card.actionNames[index], 0)
                }
            }
        }
    }
    // Close button (on hover)
    Rectangle {
        visible: hov.hovered
        x: -6; y: -6; width: 20; height: 20; radius: 10
        color: Theme.dark ? "#3a3a3c" : "#f0f0f2"
        border.width: 0.5; border.color: Theme.controlBorder
        Symbol { anchors.centerIn: parent; width: 10; height: 10; name: "x"; weight: Symbol.Bold; color: Theme.secondaryLabel }
        MouseArea { anchors.fill: parent; onClicked: { card.dismissed(); card.notifications.close(card.idx()) } }
    }
}
