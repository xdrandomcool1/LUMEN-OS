// SPDX-License-Identifier: GPL-3.0-or-later
// Weather from Open-Meteo (no API key). Only fetches after the user has set
// a location in Settings › Desktop & Dock › Widgets — no silent geolocation.
import QtQuick
import Lumen.Design
import Lumen.Shell

WidgetFrame {
    id: w
    medium: true
    property var current: null
    property var daily: null
    property string error: ""
    readonly property bool configured: ShellSettings.weatherLatitude !== 0 || ShellSettings.weatherLongitude !== 0

    function codeInfo(c) {
        if (c === 0) return ["sun", qsTr("Clear")]
        if (c <= 2) return ["cloud-sun", qsTr("Partly Cloudy")]
        if (c === 3) return ["cloud", qsTr("Cloudy")]
        if (c >= 45 && c <= 48) return ["cloud", qsTr("Fog")]
        if (c >= 51 && c <= 67) return ["cloud-rain", qsTr("Rain")]
        if (c >= 71 && c <= 77) return ["cloud-snow", qsTr("Snow")]
        if (c >= 80 && c <= 82) return ["cloud-rain", qsTr("Showers")]
        if (c >= 95) return ["cloud-lightning", qsTr("Thunderstorm")]
        return ["cloud", ""]
    }
    function refresh() {
        if (!configured) return
        const url = "https://api.open-meteo.com/v1/forecast?latitude=" + ShellSettings.weatherLatitude
                  + "&longitude=" + ShellSettings.weatherLongitude
                  + "&current=temperature_2m,weather_code&daily=weather_code,temperature_2m_max,temperature_2m_min&timezone=auto&forecast_days=5"
        const xhr = new XMLHttpRequest()
        xhr.onreadystatechange = () => {
            if (xhr.readyState !== XMLHttpRequest.DONE) return
            if (xhr.status === 200) {
                try { const j = JSON.parse(xhr.responseText); w.current = j.current; w.daily = j.daily; w.error = "" }
                catch (e) { w.error = qsTr("Weather unavailable") }
            } else w.error = qsTr("Weather unavailable")
        }
        xhr.open("GET", url); xhr.send()
    }
    Timer { interval: 20 * 60 * 1000; running: w.configured && w.visible; repeat: true; triggeredOnStart: true; onTriggered: w.refresh() }

    Column {
        visible: !w.configured
        anchors.centerIn: parent; spacing: 6
        Symbol { anchors.horizontalCenter: parent.horizontalCenter; width: 28; height: 28; name: "cloud-sun"; weight: Symbol.Fill; color: Theme.secondaryLabel }
        LText { anchors.horizontalCenter: parent.horizontalCenter; text: qsTr("Choose a location in Settings"); secondary: true; style: LText.Callout }
    }
    Item {
        visible: w.configured
        anchors.fill: parent; anchors.margins: 14
        Column {
            LText { text: ShellSettings.weatherCity.length ? ShellSettings.weatherCity : qsTr("Weather"); style: LText.Headline }
            LText { text: w.current ? Math.round(w.current.temperature_2m) + "°" : "—"; font.pixelSize: 40; font.weight: Font.Light }
            Row { spacing: 4
                Symbol { width: 14; height: 14; name: w.current ? w.codeInfo(w.current.weather_code)[0] : "cloud"; weight: Symbol.Fill; color: Theme.label }
                LText { text: w.error.length ? w.error : w.current ? w.codeInfo(w.current.weather_code)[1] : qsTr("Loading…"); style: LText.Caption }
            }
        }
        Row {
            anchors.right: parent.right; anchors.bottom: parent.bottom
            spacing: 14
            Repeater {
                model: w.daily ? Math.min(4, w.daily.time.length) : 0
                Column {
                    required property int index
                    spacing: 4
                    LText { anchors.horizontalCenter: parent.horizontalCenter; text: Qt.locale().dayName(new Date(w.daily.time[index]).getDay(), Locale.ShortFormat); style: LText.Caption; secondary: true }
                    Symbol { anchors.horizontalCenter: parent.horizontalCenter; width: 18; height: 18; name: w.codeInfo(w.daily.weather_code[index])[0]; weight: Symbol.Fill; color: Theme.label }
                    LText { anchors.horizontalCenter: parent.horizontalCenter; text: Math.round(w.daily.temperature_2m_max[index]) + "°"; style: LText.Caption; font.weight: Font.DemiBold }
                    LText { anchors.horizontalCenter: parent.horizontalCenter; text: Math.round(w.daily.temperature_2m_min[index]) + "°"; style: LText.Caption; secondary: true }
                }
            }
        }
    }
}
