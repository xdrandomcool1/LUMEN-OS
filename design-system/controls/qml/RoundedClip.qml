// SPDX-License-Identifier: GPL-3.0-or-later
// layer.effect that clips to a rounded rectangle with continuous corners.
import QtQuick
import Lumen.Design

ShaderEffect {
    property real radius: Theme.metrics.windowRadius
    property size itemSize: Qt.size(width, height)
    fragmentShader: "qrc:/lumen/shaders/roundedclip.frag.qsb"
}
