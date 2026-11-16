// SPDX-License-Identifier: GPL-3.0-or-later
// GPU blur pyramid for a backdrop item (dual-Kawase, 4 down + 4 up passes).
//
// One BackdropBlur is shared by every GlassSurface in a window: the expensive
// part (blurring the backdrop) is done once per frame where the backdrop
// changes, not once per glass surface. The source is rendered at half
// resolution; with live updates the pyramid only re-renders when the backdrop
// subtree is dirty, so a static desktop costs no GPU time.
import QtQuick

Item {
    id: root

    //! Item whose rendering is the backdrop (e.g. the wallpaper layer).
    //! Must not contain the glass surfaces that sample it.
    property Item source: null
    //! Blur spread multiplier. 1.0 ≈ 30 px radius at the source scale.
    property real strength: 1.0
    property bool live: true

    //! Blurred backdrop texture (half resolution).
    readonly property alias texture: out4
    //! Unblurred backdrop texture (for "clear" glass).
    readonly property alias sharp: level0

    readonly property real srcW: Math.max(2, (source ? source.width : 2) / 2)
    readonly property real srcH: Math.max(2, (source ? source.height : 2) / 2)

    width: 0
    height: 0
    clip: false

    ShaderEffectSource {
        id: level0
        sourceItem: root.source
        textureSize: Qt.size(Math.ceil(root.srcW), Math.ceil(root.srcH))
        live: root.live
        hideSource: false
        smooth: true
        visible: false
        wrapMode: ShaderEffectSource.ClampToEdge
    }

    component Pass: ShaderEffect {
        property var source
        property real scaleDiv: 2
        property point texel: Qt.point(1 / Math.max(1, width * scaleDiv), 1 / Math.max(1, height * scaleDiv))
        property real offset: root.strength
        // Rendered only into its ShaderEffectSource (hideSource), never on screen.
        x: 0; y: 0
    }

    component Out: ShaderEffectSource {
        live: root.live
        hideSource: true
        smooth: true
        visible: false
        wrapMode: ShaderEffectSource.ClampToEdge
    }

    // Downsample: 1/2 → 1/4 → 1/8 → 1/16 → 1/32
    Pass { id: d1; source: level0; width: Math.ceil(root.srcW / 2); height: Math.ceil(root.srcH / 2); fragmentShader: "qrc:/lumen/shaders/kawase_down.frag.qsb"; scaleDiv: 2 }
    Out { id: o1; sourceItem: d1; textureSize: Qt.size(d1.width, d1.height) }
    Pass { id: d2; source: o1; width: Math.ceil(d1.width / 2); height: Math.ceil(d1.height / 2); fragmentShader: "qrc:/lumen/shaders/kawase_down.frag.qsb"; scaleDiv: 2 }
    Out { id: o2; sourceItem: d2; textureSize: Qt.size(d2.width, d2.height) }
    Pass { id: d3; source: o2; width: Math.ceil(d2.width / 2); height: Math.ceil(d2.height / 2); fragmentShader: "qrc:/lumen/shaders/kawase_down.frag.qsb"; scaleDiv: 2 }
    Out { id: o3; sourceItem: d3; textureSize: Qt.size(d3.width, d3.height) }
    Pass { id: d4; source: o3; width: Math.ceil(d3.width / 2); height: Math.ceil(d3.height / 2); fragmentShader: "qrc:/lumen/shaders/kawase_down.frag.qsb"; scaleDiv: 2 }
    Out { id: o4; sourceItem: d4; textureSize: Qt.size(d4.width, d4.height) }

    // Upsample: 1/32 → 1/16 → 1/8 → 1/4 → 1/2
    Pass { id: u1; source: o4; width: d3.width; height: d3.height; fragmentShader: "qrc:/lumen/shaders/kawase_up.frag.qsb"; scaleDiv: 0.5 }
    Out { id: out1; sourceItem: u1; textureSize: Qt.size(u1.width, u1.height) }
    Pass { id: u2; source: out1; width: d2.width; height: d2.height; fragmentShader: "qrc:/lumen/shaders/kawase_up.frag.qsb"; scaleDiv: 0.5 }
    Out { id: out2; sourceItem: u2; textureSize: Qt.size(u2.width, u2.height) }
    Pass { id: u3; source: out2; width: d1.width; height: d1.height; fragmentShader: "qrc:/lumen/shaders/kawase_up.frag.qsb"; scaleDiv: 0.5 }
    Out { id: out3; sourceItem: u3; textureSize: Qt.size(u3.width, u3.height) }
    Pass { id: u4; source: out3; width: Math.ceil(root.srcW); height: Math.ceil(root.srcH); fragmentShader: "qrc:/lumen/shaders/kawase_up.frag.qsb"; scaleDiv: 0.5 }
    Out { id: out4; sourceItem: u4; textureSize: Qt.size(u4.width, u4.height) }
}
