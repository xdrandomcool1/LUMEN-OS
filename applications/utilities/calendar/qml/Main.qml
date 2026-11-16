// SPDX-License-Identifier: GPL-3.0-or-later
// Calendar: month view with local events (stored as JSON under XDG data).
import QtQuick
import QtCore
import Lumen.Design
import Lumen.Motion
import Lumen.Controls
import Lumen.Calendar

AppWindow {
    id: win
    property string openFile
    title: Qt.locale().standaloneMonthName(month) + " " + year
    width: 1000; height: 680
    visible: true
    property int year: new Date().getFullYear()
    property int month: new Date().getMonth()
    property date selected: new Date()
    readonly property date today: new Date()

    EventStore { id: store }

    function shift(d) { let m = month + d, y = year; while (m < 0) { m += 12; y-- } while (m > 11) { m -= 12; y++ } month = m; year = y }
    function key(d) { return Qt.formatDate(d, "yyyy-MM-dd") }

    sidebar: Column {
        spacing: 10
        Item { width: 1; height: 4 }
        LText { x: 16; text: Qt.formatDate(win.selected, "dddd d MMMM"); style: LText.Headline }
        Repeater {
            model: (store.revision, store.eventsOn(win.key(win.selected)))
            Rectangle {
                required property var modelData
                x: 10; width: parent.width - 20; height: 46; radius: 8
                color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.18)
                Rectangle { width: 4; height: parent.height - 12; y: 6; x: 6; radius: 2; color: Theme.accent }
                Column { x: 16; anchors.verticalCenter: parent.verticalCenter
                    LText { text: modelData.title; font.weight: Font.DemiBold; width: 150 }
                    LText { text: modelData.time || qsTr("All day"); secondary: true; style: LText.Caption } }
                Symbol { anchors.right: parent.right; anchors.rightMargin: 8; anchors.verticalCenter: parent.verticalCenter; width: 12; height: 12; name: "x"; color: Theme.secondaryLabel
                         MouseArea { anchors.fill: parent; anchors.margins: -6; onClicked: store.remove(modelData.id) } }
            }
        }
        LText { x: 16; visible: (store.revision, store.eventsOn(win.key(win.selected))).length === 0; text: qsTr("No events"); tertiary: true }
        Column {
            x: 10; width: parent.width - 20; spacing: 6
            TextField { id: newTitle; width: parent.width; placeholderText: qsTr("New event") }
            TextField { id: newTime; width: parent.width; placeholderText: qsTr("Time (e.g. 14:30)") }
            Button { kind: Button.Prominent; text: qsTr("Add"); enabled: newTitle.text.length > 0
                     onClicked: { store.add(win.key(win.selected), newTitle.text, newTime.text); newTitle.text = ""; newTime.text = "" } }
        }
    }
    toolbarTrailing: [
        ToolbarGroup {
            ToolbarButton { symbol: "caret-left"; onClicked: win.shift(-1) }
            ToolbarButton { text: qsTr("Today"); onClicked: { win.year = win.today.getFullYear(); win.month = win.today.getMonth(); win.selected = win.today } }
            ToolbarButton { symbol: "caret-right"; onClicked: win.shift(1) }
        }
    ]
    Item {
        id: grid
        x: 0; y: win.toolbarHeight + 4
        width: win.contentArea.width; height: win.contentArea.height - y
        readonly property int firstDow: (new Date(win.year, win.month, 1).getDay() + 6) % 7
        Row {
            id: dows
            Repeater { model: 7
                LText { required property int index; width: grid.width / 7; height: 24; horizontalAlignment: Text.AlignRight; rightPadding: 10
                        text: Qt.locale().dayName((index + 1) % 7, Locale.ShortFormat); secondary: true; style: LText.Callout } }
        }
        Grid {
            y: 26; columns: 7
            Repeater {
                model: 42
                Rectangle {
                    required property int index
                    readonly property date day: new Date(win.year, win.month, index - grid.firstDow + 1)
                    readonly property bool inMonth: day.getMonth() === win.month
                    readonly property bool isToday: win.key(day) === win.key(win.today)
                    readonly property bool isSel: win.key(day) === win.key(win.selected)
                    width: grid.width / 7; height: (grid.height - 26) / 6
                    color: isSel ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.10) : "transparent"
                    border.width: 0.5; border.color: Theme.separator
                    Rectangle { anchors.right: parent.right; anchors.top: parent.top; anchors.margins: 6; width: 26; height: 22; radius: 11; color: Theme.red; visible: parent.isToday }
                    LText { anchors.right: parent.right; anchors.top: parent.top; anchors.margins: 6; width: 26; height: 22; horizontalAlignment: Text.AlignHCenter
                            text: parent.day.getDate(); color: parent.isToday ? "white" : parent.inMonth ? Theme.label : Theme.tertiaryLabel; font.weight: parent.isToday ? Font.Bold : Font.Normal }
                    Column {
                        x: 6; y: 32; width: parent.width - 12; spacing: 2
                        Repeater { model: (store.revision, store.eventsOn(win.key(parent.parent.day))).slice(0, 3)
                            Rectangle { required property var modelData; width: parent.width; height: 18; radius: 4; color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.22)
                                LText { x: 4; anchors.verticalCenter: parent.verticalCenter; width: parent.width - 8; text: (modelData.time ? modelData.time + " " : "") + modelData.title; style: LText.Caption } } }
                    }
                    MouseArea { anchors.fill: parent; onClicked: win.selected = parent.day }
                }
            }
        }
    }
}
