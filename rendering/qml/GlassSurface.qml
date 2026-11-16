// SPDX-License-Identifier: GPL-3.0-or-later
// A Liquid Glass surface. Children are placed on top of the glass.
//
//   GlassSurface {
//       backdrop: desktopBlur          // BackdropBlur (in-scene) or null
//       material: Glass.Regular
//       radius: 22
//       Text { ... }
//   }
//
// With a backdrop the full effect is rendered in-process (refraction,
// dispersion, adaptive tint). Without one (cross-surface backdrop), the
// surface renders tint + light only and expects the compositor to blur
// behind it (see GlassWindow helpers in lumen-shell).
import QtQuick
import Lumen.Design
import Lumen.Glass

Item {
    id: root

    property int material: Glass.Regular
    property BackdropBlur backdrop: null
    property bool blurred: true
    property real radius: 18
    //! Corner exponent: 2 = circular arcs, 2.6 ≈ continuous corners.
    property real smoothness: 2.6
    property bool shadowEnabled: true
    property real shadowScale: 1.0
    property real shadowOffsetY: 6
    property real refractionScale: 1.0
    property real specularScale: 1.0
    //! 0..1 press glow (bind to a Spring for interactive glass).
    property real pressed: 0
    //! Optional tint override; alpha < 0 keeps the material tint.
    property color tintColor: "transparent"
    property real tintAlpha: -1
    property vector2d lightDirection: Qt.vector2d(0.38, 0.92).normalized()

    default property alias contentData: content.data
    readonly property alias contentItem: content
    readonly property var params: Glass.material(material, Appearance.dark, Appearance.accent)

    readonly property bool _opaqueFallback: Appearance.reduceTransparency
    readonly property real _shadowRadius: shadowEnabled ? params.shadowRadius * shadowScale : 0
    readonly property real _pad: Math.ceil(_shadowRadius * 1.6 + Math.abs(shadowOffsetY) + 2)

    BackdropTracker {
        id: tracker
        item: fx
        reference: root.backdrop ? root.backdrop.source : null
    }

    Rectangle {
        id: dummyBackdrop
        width: 1; height: 1
        visible: false
        color: "transparent"
        layer.enabled: !root.backdrop
    }

    ShaderEffect {
        id: fx
        x: -root._pad
        y: -root._pad
        width: root.width + root._pad * 2
        height: root.height + root._pad * 2
        visible: root.width > 0 && root.height > 0
        fragmentShader: "qrc:/lumen/shaders/glass.frag.qsb"
        blending: true

        readonly property bool inScene: root.backdrop !== null && !root._opaqueFallback
        property var backdrop: root.backdrop ? (root.blurred ? root.backdrop.texture : root.backdrop.sharp) : dummyBackdrop
        property size itemSize: Qt.size(width, height)
        property rect shape: Qt.rect(root._pad, root._pad, root.width, root.height)
        property real radius: root.radius
        property real smoothness: root.smoothness
        property rect backdropRect: tracker.normalizedRect
        property real hasBackdrop: inScene ? 1 : 0
        property real refraction: root.params.refraction * root.refractionScale * Appearance.glassIntensity
        property real lensWidth: root.params.lensWidth
        property real dispersion: root.params.dispersion
        property vector4d tint: {
            let t = root.params.tint
            let a = root.tintAlpha >= 0 ? root.tintAlpha : t.w
            if (root.tintColor.a > 0)
                t = Qt.vector4d(root.tintColor.r, root.tintColor.g, root.tintColor.b, a)
            else
                t = Qt.vector4d(t.x, t.y, t.z, a)
            if (root._opaqueFallback)
                return Qt.vector4d(t.x, t.y, t.z, 0.97)
            if (Appearance.increaseContrast)
                return Qt.vector4d(t.x, t.y, t.z, Math.min(1, t.w + 0.25))
            return t
        }
        property real adaptivity: root.params.adaptivity
        property real saturation: root.params.saturation
        property real brightness: root.params.brightness
        property real specular: root.params.specular * root.specularScale * Math.min(1.2, Appearance.glassIntensity)
        property vector2d lightDir: root.lightDirection
        property vector4d shadowParams: Qt.vector4d(root.shadowEnabled ? root.params.shadow : 0,
                                                    root._shadowRadius, root.shadowOffsetY, 0)
        property real outline: Appearance.increaseContrast ? 0.35 : 0
        property real dark: Appearance.dark ? 1 : 0
        property real pressed: root.pressed
    }

    Item {
        id: content
        anchors.fill: parent
    }
}
