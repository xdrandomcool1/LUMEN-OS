// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import Lumen.Design
import Lumen.Shell
import Lumen.System

WidgetFrame {
    id: w
    property bool small: true
    // Analog clock face.
    Canvas {
        id: face
        anchors.centerIn: parent
        width: 132; height: 132
        property date t: ShellState.now
        onTChanged: requestPaint()
        onPaint: {
            const c = getContext("2d"); const r = width / 2
            c.reset(); c.translate(r, r)
            c.fillStyle = Theme.dark ? "rgba(255,255,255,0.06)" : "rgba(255,255,255,0.55)"
            c.beginPath(); c.arc(0, 0, r - 1, 0, 2 * Math.PI); c.fill()
            c.fillStyle = Theme.label.toString()
            c.font = "600 13px 'Inter Variable'"; c.textAlign = "center"; c.textBaseline = "middle"
            for (let h = 1; h <= 12; ++h) {
                const a = h * Math.PI / 6
                c.fillText(String(h), Math.sin(a) * (r - 14), -Math.cos(a) * (r - 14))
            }
            const hr = t.getHours() % 12 + t.getMinutes() / 60, mn = t.getMinutes() + t.getSeconds() / 60, sc = t.getSeconds()
            function hand(angle, len, w, col) {
                c.save(); c.rotate(angle); c.strokeStyle = col; c.lineWidth = w; c.lineCap = "round"
                c.beginPath(); c.moveTo(0, 6); c.lineTo(0, -len); c.stroke(); c.restore()
            }
            hand(hr * Math.PI / 6, r * 0.48, 4, Theme.label.toString())
            hand(mn * Math.PI / 30, r * 0.72, 3, Theme.label.toString())
            hand(sc * Math.PI / 30, r * 0.78, 1.2, "#FF9F0A")
            c.fillStyle = "#FF9F0A"; c.beginPath(); c.arc(0, 0, 3, 0, 2 * Math.PI); c.fill()
        }
    }
}
