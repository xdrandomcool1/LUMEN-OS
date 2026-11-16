// SPDX-License-Identifier: GPL-3.0-or-later
// Text with LUMEN typography roles.
import QtQuick
import Lumen.Design

Text {
    id: t
    enum Style { LargeTitle, Title1, Title2, Title3, Headline, Body, Callout, Subheadline, Caption, Caption2 }
    property int style: LText.Body
    property bool secondary: false
    property bool tertiary: false

    readonly property var _spec: {
        switch (style) {
        case LText.LargeTitle: return [Theme.type.largeTitle, Font.Bold, 0.22]
        case LText.Title1: return [Theme.type.title1, Font.Bold, -0.26]
        case LText.Title2: return [Theme.type.title2, Font.DemiBold, -0.43]
        case LText.Title3: return [Theme.type.title3, Font.DemiBold, -0.23]
        case LText.Headline: return [Theme.type.headline, Font.DemiBold, -0.08]
        case LText.Callout: return [Theme.type.callout, Font.Normal, 0]
        case LText.Subheadline: return [Theme.type.subheadline, Font.Normal, 0.06]
        case LText.Caption: return [Theme.type.caption, Font.Normal, 0.06]
        case LText.Caption2: return [Theme.type.caption2, Font.Medium, 0.07]
        default: return [Theme.type.body, Font.Normal, -0.08]
        }
    }

    color: tertiary ? Theme.tertiaryLabel : secondary ? Theme.secondaryLabel : Theme.label
    font.family: Theme.fontFamily
    font.pixelSize: _spec[0]
    font.weight: _spec[1]
    font.letterSpacing: _spec[2]
    font.hintingPreference: Font.PreferNoHinting
    renderType: Text.QtRendering
    elide: Text.ElideRight
    verticalAlignment: Text.AlignVCenter
}
