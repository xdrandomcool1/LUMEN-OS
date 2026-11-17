// SPDX-License-Identifier: GPL-3.0-or-later
// Glass frame shared by all widgets (small 164², medium 344×164).
import QtQuick
import Lumen.Design
import Lumen.Glass

GlassSurface {
    property bool medium: false
    width: medium ? 344 : 164
    height: 164
    radius: Theme.metrics.widgetRadius
    material: Glass.Regular
    shadowScale: 0.7
    shadowOffsetY: 6
}
