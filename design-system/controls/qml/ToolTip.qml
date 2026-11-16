// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Templates as T
import Lumen.Design
import Lumen.Motion
import Lumen.Glass

T.ToolTip {
    id: control
    delay: Motion.tooltipDelay
    padding: 6
    leftPadding: 9; rightPadding: 9
    margins: 6
    y: -implicitHeight - 6
    contentItem: LText { text: control.text; style: LText.Callout; wrapMode: Text.Wrap; elide: Text.ElideNone }
    background: GlassSurface {
        radius: 8; material: Glass.Thin
        backdrop: (Window.window && Window.window.lumenBackdrop) ? Window.window.lumenBackdrop : null
        shadowScale: 0.5; shadowOffsetY: 3
    }
    enter: Transition { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Motion.fast } }
    exit: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: Motion.instant } }
}
