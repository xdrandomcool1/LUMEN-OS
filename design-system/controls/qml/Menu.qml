// SPDX-License-Identifier: GPL-3.0-or-later
// Context / pull-down menu on thin Liquid Glass. Opens with a quick scale +
// fade from its anchor point, closes with a faster fade.
import QtQuick
import QtQuick.Templates as T
import Lumen.Design
import Lumen.Motion
import Lumen.Glass

T.Menu {
    id: control
    property BackdropBlur backdrop: (control.parent && control.parent.Window.window && control.parent.Window.window.lumenBackdrop) ? control.parent.Window.window.lumenBackdrop : null
    property real minimumWidth: 180

    implicitWidth: Math.max(minimumWidth, contentWidth + leftPadding + rightPadding)
    implicitHeight: contentHeight + topPadding + bottomPadding
    padding: Theme.metrics.menuInset
    margins: 6
    overlap: 4
    transformOrigin: Item.TopLeft

    delegate: MenuItem {}

    contentItem: ListView {
        implicitHeight: contentHeight
        implicitWidth: {
            let w = 0
            for (let i = 0; i < count; ++i) {
                const it = itemAtIndex(i)
                if (it) w = Math.max(w, it.implicitWidth)
            }
            return w
        }
        model: control.contentModel
        interactive: Window.window ? contentHeight + control.topPadding + control.bottomPadding > Window.window.height : false
        clip: true
        currentIndex: control.currentIndex
        boundsBehavior: Flickable.StopAtBounds
    }

    background: GlassSurface {
        implicitWidth: 180
        implicitHeight: 30
        radius: Theme.metrics.menuRadius
        material: Glass.Thin
        backdrop: control.backdrop
        shadowScale: 0.9
        shadowOffsetY: 8
    }

    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Motion.fast; easing.type: Easing.Bezier; easing.bezierCurve: Motion.decelerate }
        NumberAnimation { property: "scale"; from: 0.96; to: 1; duration: Motion.fast; easing.type: Easing.Bezier; easing.bezierCurve: Motion.emphasized }
    }
    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1; to: 0; duration: Motion.instant + 30; easing.type: Easing.Bezier; easing.bezierCurve: Motion.exit }
    }
}
