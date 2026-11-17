// SPDX-License-Identifier: GPL-3.0-or-later
// LUMEN design tokens. See docs/DESIGN_SYSTEM.md for rationale.
pragma Singleton
import QtQuick
import Lumen.Design

QtObject {
    id: theme

    readonly property bool dark: Appearance.dark
    readonly property color accent: Appearance.accent

    // ---- Typography --------------------------------------------------------
    readonly property string fontFamily: "Inter Variable"
    readonly property string fallbackFamily: "Inter"
    readonly property string displayFamily: "Inter Display"
    readonly property string monoFamily: "JetBrains Mono"

    readonly property QtObject type: QtObject {
        readonly property real largeTitle: 26
        readonly property real title1: 22
        readonly property real title2: 17
        readonly property real title3: 15
        readonly property real headline: 13
        readonly property real body: 13
        readonly property real callout: 12
        readonly property real subheadline: 11
        readonly property real caption: 11
        readonly property real caption2: 10
        readonly property real menuBar: 13
    }

    // ---- Geometry ----------------------------------------------------------
    readonly property QtObject metrics: QtObject {
        readonly property real menuBarHeight: 24
        readonly property real menuRadius: 12
        readonly property real menuItemHeight: 22
        readonly property real menuItemRadius: 6
        readonly property real menuInset: 5
        readonly property real dockTile: 64
        readonly property real dockIcon: 54
        readonly property real dockPadding: 6
        readonly property real dockGap: 8
        readonly property real windowRadius: 16
        readonly property real toolbarWindowRadius: 24
        readonly property real titleBarHeight: 32
        readonly property real toolbarHeight: 52
        readonly property real trafficLight: 12
        readonly property real trafficSpacing: 20
        readonly property real trafficInset: 14
        readonly property real sidebarWidth: 220
        readonly property real controlRadius: 6
        readonly property real controlHeight: 22
        readonly property real largeControlHeight: 28
        readonly property real panelRadius: 26
        readonly property real moduleRadius: 20
        readonly property real widgetRadius: 22
        readonly property real spacing: 8
        readonly property real padding: 12
    }

    // ---- Semantic colors ---------------------------------------------------
    function a(c, alpha) { return Qt.rgba(c.r, c.g, c.b, alpha) }

    readonly property color label: dark ? Qt.rgba(1, 1, 1, 0.88) : Qt.rgba(0, 0, 0, 0.86)
    readonly property color secondaryLabel: dark ? Qt.rgba(1, 1, 1, 0.55) : Qt.rgba(0, 0, 0, 0.50)
    readonly property color tertiaryLabel: dark ? Qt.rgba(1, 1, 1, 0.25) : Qt.rgba(0, 0, 0, 0.26)
    readonly property color quaternaryLabel: dark ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(0, 0, 0, 0.10)
    readonly property color separator: dark ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(0, 0, 0, 0.10)
    readonly property color windowBackground: dark ? "#1E1E20" : "#F5F5F7"
    readonly property color contentBackground: dark ? "#161618" : "#FFFFFF"
    readonly property color sidebarBackground: dark ? "#262628" : "#EDEDF0"
    readonly property color controlFill: dark ? Qt.rgba(1, 1, 1, 0.10) : Qt.rgba(0, 0, 0, 0.05)
    readonly property color controlFillHover: dark ? Qt.rgba(1, 1, 1, 0.16) : Qt.rgba(0, 0, 0, 0.09)
    readonly property color controlFillPressed: dark ? Qt.rgba(1, 1, 1, 0.22) : Qt.rgba(0, 0, 0, 0.14)
    readonly property color controlBackground: dark ? "#3A3A3C" : "#FFFFFF"
    readonly property color controlBorder: dark ? Qt.rgba(1, 1, 1, 0.10) : Qt.rgba(0, 0, 0, 0.12)
    readonly property color selection: a(accent, dark ? 0.85 : 0.90)
    readonly property color selectionInactive: dark ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(0, 0, 0, 0.08)
    readonly property color textOnAccent: "#FFFFFF"
    readonly property color link: accent
    readonly property color focusRing: a(accent, 0.55)
    readonly property color menuHighlight: a(accent, 0.92)

    // Status colors
    readonly property color red: dark ? "#FF453A" : "#FF3B30"
    readonly property color orange: dark ? "#FF9F0A" : "#FF9500"
    readonly property color yellow: dark ? "#FFD60A" : "#FFCC00"
    readonly property color green: dark ? "#30D158" : "#34C759"
    readonly property color mint: dark ? "#63E6E2" : "#00C7BE"
    readonly property color teal: dark ? "#40C8E0" : "#30B0C7"
    readonly property color cyan: dark ? "#64D2FF" : "#32ADE6"
    readonly property color blue: dark ? "#0A84FF" : "#007AFF"
    readonly property color indigo: dark ? "#5E5CE6" : "#5856D6"
    readonly property color purple: dark ? "#BF5AF2" : "#AF52DE"
    readonly property color pink: dark ? "#FF375F" : "#FF2D55"
    readonly property color gray: dark ? "#98989D" : "#8E8E93"

    // Window controls ("traffic lights")
    readonly property color closeButton: "#FF5F57"
    readonly property color minimizeButton: "#FEBC2E"
    readonly property color zoomButton: "#28C840"
    readonly property color inactiveButton: dark ? "#4A4A4D" : "#D0D0D2"

    // Text over glass/wallpaper (menu bar): always light with soft shadow.
    readonly property color onWallpaper: Qt.rgba(1, 1, 1, 0.96)
}
