// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import Lumen.Design
import Lumen.Shell

WidgetFrame {
    id: w
    readonly property date today: ShellState.now
    readonly property int year: today.getFullYear()
    readonly property int month: today.getMonth()
    readonly property int firstDow: (new Date(year, month, 1).getDay() + 6) % 7 // Monday first
    readonly property int days: new Date(year, month + 1, 0).getDate()

    Column {
        x: 12; y: 10
        spacing: 3
        LText { text: Qt.locale().monthName(w.month, Locale.LongFormat).toUpperCase(); color: Theme.red; style: LText.Caption; font.weight: Font.Bold }
        Grid {
            columns: 7; columnSpacing: 2; rowSpacing: 1
            Repeater {
                model: 7
                LText { required property int index; width: 18; horizontalAlignment: Text.AlignHCenter
                        text: Qt.locale().dayName((index + 1) % 7, Locale.ShortFormat).charAt(0).toUpperCase(); style: LText.Caption2; secondary: true }
            }
            Repeater {
                model: 42
                Item {
                    required property int index
                    readonly property int day: index - w.firstDow + 1
                    readonly property bool valid: day >= 1 && day <= w.days
                    readonly property bool isToday: valid && day === w.today.getDate()
                    width: 18; height: 15
                    visible: index < Math.ceil((w.firstDow + w.days) / 7) * 7
                    Rectangle { anchors.centerIn: parent; width: 17; height: 15; radius: 7.5; color: Theme.red; visible: parent.isToday }
                    LText { anchors.centerIn: parent; text: parent.valid ? parent.day : ""; style: LText.Caption2
                            color: parent.isToday ? "white" : Theme.label; font.weight: parent.isToday ? Font.Bold : Font.Medium }
                }
            }
        }
    }
    MouseArea { anchors.fill: parent; onClicked: AppsModel.launch("lumen-calendar") }
}
