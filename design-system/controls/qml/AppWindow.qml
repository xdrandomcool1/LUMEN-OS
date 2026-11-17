// SPDX-License-Identifier: GPL-3.0-or-later
// Top-level window for LUMEN applications.
//
// Draws its own chrome so that apps get the unified-toolbar layout: traffic
// lights over a floating Liquid Glass sidebar, glass toolbar groups, content
// scrolling under the toolbar, continuous rounded corners and a compositor
// drop shadow. Move/resize are delegated to the compositor
// (startSystemMove/startSystemResize), so snapping, tiling and gestures work
// exactly as for server-side-decorated windows.
import QtQuick
import QtQuick.Window
import Lumen.Design
import Lumen.Motion
import Lumen.Glass
import Lumen.Controls

Window {
    id: win

    property Component sidebar: null
    property real sidebarWidth: Theme.metrics.sidebarWidth
    property bool sidebarVisible: sidebar !== null
    property string subtitle: ""
    property bool showTitle: true
    property real toolbarHeight: Theme.metrics.toolbarHeight
    property color contentColor: Theme.contentBackground
    property alias toolbarLeading: leadingRow.data
    property alias toolbarTrailing: trailingRow.data
    default property alias contentData: contentArea.data
    readonly property alias contentArea: contentArea
    readonly property alias lumenBackdrop: blur
    readonly property bool maximized: visibility === Window.Maximized || visibility === Window.FullScreen
    readonly property real cornerRadius: maximized ? 0 : Theme.metrics.windowRadius
    readonly property real sidebarInset: 8

    flags: Qt.Window | Qt.FramelessWindowHint
    color: "transparent"
    width: 980
    height: 640
    minimumWidth: 520
    minimumHeight: 360

    function toggleZoom() {
        if (win.visibility === Window.Maximized) win.showNormal(); else win.showMaximized()
    }

    WindowEffects {
        window: win
        radius: win.cornerRadius
        active: win.active
        shadow: !win.maximized
    }

    Spring { id: sidebarSpring; to: win.sidebarVisible ? 1 : 0; preset: Spring.Smooth }
    readonly property real _sidebarSpan: (win.sidebarWidth + win.sidebarInset) * sidebarSpring.value

    Item {
        id: frame
        anchors.fill: parent
        layer.enabled: win.cornerRadius > 0
        layer.effect: RoundedClip { radius: win.cornerRadius }

        // ---- Backdrop layer: everything glass may refract -----------------
        Item {
            id: backdropLayer
            anchors.fill: parent
            Rectangle { anchors.fill: parent; color: Theme.windowBackground }
            Rectangle {
                x: win._sidebarSpan; width: parent.width - x; height: parent.height
                color: win.contentColor
            }
            Item {
                id: contentArea
                x: win._sidebarSpan
                width: parent.width - x
                height: parent.height
            }
        }

        BackdropBlur { id: blur; source: backdropLayer }

        // Scroll-edge effect: content softly fades under the toolbar.
        Rectangle {
            x: contentArea.x; width: contentArea.width; height: win.toolbarHeight + 10
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.rgba(win.contentColor.r, win.contentColor.g, win.contentColor.b, 0.92) }
                GradientStop { position: 0.65; color: Qt.rgba(win.contentColor.r, win.contentColor.g, win.contentColor.b, 0.55) }
                GradientStop { position: 1.0; color: Qt.rgba(win.contentColor.r, win.contentColor.g, win.contentColor.b, 0.0) }
            }
        }

        // ---- Floating glass sidebar --------------------------------------
        GlassSurface {
            id: sidebarGlass
            visible: sidebarSpring.value > 0.01
            x: win.sidebarInset - (1 - sidebarSpring.value) * (win.sidebarWidth + win.sidebarInset)
            y: win.sidebarInset
            width: win.sidebarWidth
            height: parent.height - 2 * win.sidebarInset
            radius: Math.max(10, win.cornerRadius - win.sidebarInset)
            material: Glass.Frosted
            backdrop: blur
            shadowScale: 0.5
            shadowOffsetY: 2
            Loader {
                anchors.fill: parent
                anchors.topMargin: win.toolbarHeight - win.sidebarInset
                sourceComponent: win.sidebar
            }
        }

        // ---- Unified toolbar ----------------------------------------------
        Item {
            id: toolbar
            x: 0; width: parent.width; height: win.toolbarHeight

            DragHandler {
                target: null
                onActiveChanged: if (active) win.startSystemMove()
            }
            TapHandler {
                onDoubleTapped: win.toggleZoom()
            }

            Row {
                id: leadingRow
                x: Math.max(contentArea.x + 12, traffic.x + traffic.width + 20)
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8
            }
            Column {
                visible: win.showTitle
                x: leadingRow.x + leadingRow.width + (leadingRow.width > 0 ? 12 : 0)
                anchors.verticalCenter: parent.verticalCenter
                LText { text: win.title; style: LText.Title3; font.weight: Font.Bold }
                LText { visible: win.subtitle.length > 0; text: win.subtitle; style: LText.Subheadline; secondary: true }
            }
            Row {
                id: trailingRow
                anchors.right: parent.right; anchors.rightMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8
            }
        }

        TrafficLights {
            id: traffic
            x: 20
            y: (win.toolbarHeight - Theme.metrics.trafficLight) / 2
            active: win.active
            onCloseRequested: win.close()
            onMinimizeRequested: win.showMinimized()
            onZoomRequested: win.toggleZoom()
        }

        // Hairline highlight around the window edge (glass rim of the frame).
        Rectangle {
            anchors.fill: parent
            radius: win.cornerRadius
            color: "transparent"
            border.width: 1
            border.color: Theme.dark ? Qt.rgba(1, 1, 1, 0.10) : Qt.rgba(1, 1, 1, 0.5)
            visible: !win.maximized
        }
    }

    // ---- Resize edges (handled by the compositor) ---------------------------
    component Edge: MouseArea {
        property int edges
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        onPressed: win.startSystemResize(edges)
        enabled: !win.maximized
    }
    Edge { edges: Qt.LeftEdge; x: 0; y: 8; width: 5; height: parent.height - 16; cursorShape: Qt.SizeHorCursor }
    Edge { edges: Qt.RightEdge; x: parent.width - 5; y: 8; width: 5; height: parent.height - 16; cursorShape: Qt.SizeHorCursor }
    Edge { edges: Qt.TopEdge; x: 8; y: 0; width: parent.width - 16; height: 4; cursorShape: Qt.SizeVerCursor }
    Edge { edges: Qt.BottomEdge; x: 8; y: parent.height - 5; width: parent.width - 16; height: 5; cursorShape: Qt.SizeVerCursor }
    Edge { edges: Qt.TopEdge | Qt.LeftEdge; x: 0; y: 0; width: 10; height: 10; cursorShape: Qt.SizeFDiagCursor }
    Edge { edges: Qt.TopEdge | Qt.RightEdge; x: parent.width - 10; y: 0; width: 10; height: 10; cursorShape: Qt.SizeBDiagCursor }
    Edge { edges: Qt.BottomEdge | Qt.LeftEdge; x: 0; y: parent.height - 10; width: 10; height: 10; cursorShape: Qt.SizeBDiagCursor }
    Edge { edges: Qt.BottomEdge | Qt.RightEdge; x: parent.width - 10; y: parent.height - 10; width: 10; height: 10; cursorShape: Qt.SizeFDiagCursor }
}
