// SPDX-License-Identifier: GPL-3.0-or-later
// Notification Center: notification history + widgets.
import QtQuick
import Lumen.Design
import Lumen.Motion
import Lumen.Glass
import Lumen.Controls
import Lumen.Shell
import Lumen.System
import Lumen.Shell.Plasma

Item {
    id: nc
    property bool shown: false
    property BackdropBlur backdrop: null
    property bool compositorBlur: false
    width: 368

    Spring { id: appear; to: nc.shown ? 1 : 0; preset: Spring.Smooth }
    visible: appear.value > 0.005
    opacity: Math.min(1, appear.value * 1.3)
    transform: Translate { x: (1 - appear.value) * 60 }
    MouseArea { anchors.fill: flick; onPressed: m => m.accepted = true }

    Notifications {
        id: history
        showExpired: true
        showDismissed: true
        showJobs: false
        groupMode: Notifications.GroupDisabled
        sortMode: Notifications.SortByDate
        sortOrder: Qt.DescendingOrder
    }

    Flickable {
        id: flick
        width: parent.width
        height: Math.min(parent.height, column.implicitHeight)
        contentHeight: column.implicitHeight
        clip: true
        boundsBehavior: Flickable.DragAndOvershootBounds
        Column {
            id: column
            width: flick.width
            spacing: 10

            Item {
                width: parent.width; height: 26
                visible: history.count > 0
                LText { anchors.verticalCenter: parent.verticalCenter; leftPadding: 6; text: qsTr("Notifications"); style: LText.Title3; color: "white" }
                Button { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; kind: Button.Glass; size: Button.Small
                         text: qsTr("Clear All"); backdrop: nc.backdrop; onClicked: history.clear(Notifications.ClearExpired) }
            }
            Repeater {
                model: history
                delegate: NotificationCard {
                    required property int index
                    required property var model
                    width: column.width
                    notifications: history
                    row: index
                    backdrop: nc.backdrop
                    compositorBlur: nc.compositorBlur
                    appIcon: model.applicationIconName || ""
                    summary: model.summary || ""
                    body: model.body || ""
                    created: model.created
                    actionNames: model.expired ? [] : (model.actionNames || [])
                    actionLabels: model.actionLabels || []
                    hasDefaultAction: model.hasDefaultAction === true && !model.expired
                }
            }
            Row {
                spacing: 12
                CalendarWidget { backdrop: nc.backdrop; compositorBlur: nc.compositorBlur }
                ClockWidget { backdrop: nc.backdrop; compositorBlur: nc.compositorBlur; small: true }
            }
            WeatherWidget { backdrop: nc.backdrop; compositorBlur: nc.compositorBlur; width: column.width }
            SystemWidget { backdrop: nc.backdrop; compositorBlur: nc.compositorBlur; width: column.width }
        }
    }
}
