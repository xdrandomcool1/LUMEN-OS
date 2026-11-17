// SPDX-License-Identifier: GPL-3.0-or-later
// A rounded module bubble inside Control Center.
import QtQuick
import Lumen.Design

Rectangle {
    default property alias content: inner.data
    radius: Theme.metrics.moduleRadius
    color: Theme.dark ? Qt.rgba(1, 1, 1, 0.07) : Qt.rgba(1, 1, 1, 0.45)
    border.width: 0.5
    border.color: Theme.dark ? Qt.rgba(1, 1, 1, 0.09) : Qt.rgba(1, 1, 1, 0.7)
    Item { id: inner; anchors.fill: parent; anchors.margins: 12 }
}
