// SPDX-License-Identifier: GPL-3.0-or-later
// Incoming notification banners (top-right), honouring Focus.
import QtQuick
import QtQuick.Window
import Lumen.Design
import Lumen.Motion
import Lumen.Controls
import Lumen.Shell
import Lumen.Shell.Plasma

Window {
    id: win
    property var targetScreen
    screen: targetScreen
    visible: false
    color: "transparent"
    title: "lumen-notifications"
    width: 380
    height: Math.max(1, stack.height + 30)

    NotificationSettings { id: settings }
    readonly property bool inhibited: settings.notificationsInhibitedUntil > ShellState.now
    readonly property bool hasBanners: popups.count > 0 && !inhibited && ShellController.activePanel !== "notifications"

    LayerSurface {
        window: win
        screen: win.targetScreen
        layer: LayerSurface.Overlay
        anchors: LayerSurface.AnchorTop | LayerSurface.AnchorRight
        marginTop: Theme.metrics.menuBarHeight + 4
        marginRight: 2
        scope: "lumen-notifications"
        inputRegion: Qt.rect(stack.x - 8, 0, stack.width + 16, stack.height + 12)
    }
    onHasBannersChanged: visible = hasBanners

    Notifications {
        id: popups
        showExpired: false
        showDismissed: false
        showJobs: false
        showNotifications: true
        showAddedDuringInhibition: false
        limit: 4
        groupMode: Notifications.GroupDisabled
        sortMode: Notifications.SortByDate
        sortOrder: Qt.DescendingOrder
        urgencies: Notifications.LowUrgency | Notifications.NormalUrgency | Notifications.CriticalUrgency
    }

    Column {
        id: stack
        x: 14; y: 8
        spacing: 8
        move: Transition { NumberAnimation { properties: "y"; duration: Motion.normal; easing.type: Easing.OutCubic } }
        add: Transition {
            NumberAnimation { property: "x"; from: 380; duration: Motion.slow; easing.type: Easing.Bezier; easing.bezierCurve: Motion.emphasized }
            NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Motion.fast }
        }
        Repeater {
            model: popups
            delegate: NotificationCard {
                required property int index
                required property var model
                notifications: popups
                row: index
                appName: model.applicationName || ""
                appIcon: model.applicationIconName || ""
                summary: model.summary || ""
                body: model.body || ""
                created: model.created
                actionNames: model.actionNames || []
                actionLabels: model.actionLabels || []
                hasDefaultAction: model.hasDefaultAction === true
                Component.onCompleted: popups.startTimeout(popups.index(index, 0))
            }
        }
    }
}
