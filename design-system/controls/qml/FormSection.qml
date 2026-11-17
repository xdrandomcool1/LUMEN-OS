// SPDX-License-Identifier: GPL-3.0-or-later
// Grouped inset list (Settings style). Children are FormRow items.
import QtQuick
import Lumen.Design

Column {
    id: root
    property string title: ""
    property string footer: ""
    default property alias rows: box.data
    spacing: 6
    width: parent ? parent.width : 400

    LText { visible: root.title.length > 0; text: root.title; style: LText.Headline; leftPadding: 4 }
    Rectangle {
        width: root.width
        height: box.implicitHeight
        radius: Theme.metrics.cardRadius
        color: Theme.dark ? Qt.rgba(1, 1, 1, 0.045) : Qt.rgba(1, 1, 1, 0.75)
        border.width: 0.5
        border.color: Theme.dark ? Qt.rgba(1, 1, 1, 0.07) : Qt.rgba(0, 0, 0, 0.07)
        Column { id: box; width: parent.width }
    }
    LText { visible: root.footer.length > 0; text: root.footer; style: LText.Callout; secondary: true; wrapMode: Text.Wrap; width: root.width; leftPadding: 4 }
}
