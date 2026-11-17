// SPDX-License-Identifier: GPL-3.0-or-later
// A capsule of Liquid Glass grouping related toolbar buttons.
import QtQuick
import Lumen.Design
import Lumen.Glass

GlassSurface {
    id: g
    default property alias items: row.data
    implicitWidth: row.implicitWidth + 8
    implicitHeight: 34
    radius: height / 2
    material: Glass.Floating
    backdrop: (Window.window && Window.window.lumenBackdrop) ? Window.window.lumenBackdrop : null
    shadowScale: 0.45
    shadowOffsetY: 2
    Row { id: row; anchors.centerIn: parent; spacing: 0 }
}
