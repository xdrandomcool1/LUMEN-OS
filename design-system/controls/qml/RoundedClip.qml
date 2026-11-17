// SPDX-License-Identifier: GPL-3.0-or-later
// layer.effect that clips to a rounded rectangle with continuous corners.
import QtQuick

ShaderEffect {
    property real radius: 16
    property size itemSize: Qt.size(width, height)
    fragmentShader: "qrc:/lumen/shaders/roundedclip.frag.qsb"
}
