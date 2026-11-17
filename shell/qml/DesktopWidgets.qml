// SPDX-License-Identifier: GPL-3.0-or-later
// Widgets on the desktop (configurable list in Settings).
import QtQuick
import Lumen.Glass
import Lumen.Shell
import Lumen.System

Item {
    id: root
    property BackdropBlur backdrop: null
    Flow {
        x: 24; y: 10
        width: 344
        spacing: 16
        Repeater {
            model: ShellSettings.desktopWidgets
            delegate: Loader {
                required property string modelData
                sourceComponent: modelData === "clock" ? clock : modelData === "calendar" ? calendar
                               : modelData === "weather" ? weather : modelData === "system" ? system : null
            }
        }
    }
    Component { id: clock; ClockWidget { backdrop: root.backdrop } }
    Component { id: calendar; CalendarWidget { backdrop: root.backdrop } }
    Component { id: weather; WeatherWidget { backdrop: root.backdrop } }
    Component { id: system; SystemWidget { backdrop: root.backdrop } }
}
