// SPDX-License-Identifier: GPL-3.0-or-later
// Calculator: basic + scientific, keyboard input, glass keypad.
import QtQuick
import Lumen.Design
import Lumen.Motion
import Lumen.Glass
import Lumen.Controls
import "engine.js" as Engine

AppWindow {
    id: win
    property string openFile
    title: ""
    showTitle: false
    width: scientific ? 560 : 260; height: 420
    minimumWidth: width; maximumWidth: width; minimumHeight: 420
    toolbarHeight: 36
    visible: true
    contentColor: Theme.dark ? "#1C1C1E" : "#ECECEE"
    property bool scientific: false
    property var st: Engine.initial()
    function press(k) { st = Engine.press(st, k) }

    toolbarTrailing: [ ToolbarButton { symbol: "calculator"; checked: win.scientific; tooltip: qsTr("Scientific"); onClicked: win.scientific = !win.scientific } ]

    Text {
        x: 16; y: 40; width: parent.width - 32; height: 70
        text: Engine.display(win.st)
        color: Theme.label
        font.family: Theme.fontFamily; font.weight: Font.Light
        font.pixelSize: Math.min(56, 56 * 10 / Math.max(10, text.length))
        horizontalAlignment: Text.AlignRight; verticalAlignment: Text.AlignBottom
        renderType: Text.QtRendering
    }
    readonly property var basic: ["AC", "±", "%", "÷", "7", "8", "9", "×", "4", "5", "6", "−", "1", "2", "3", "+", "⌫", "0", ".", "="]
    readonly property var sci: ["(", ")", "x²", "xʸ", "√", "sin", "cos", "tan", "ln", "log", "π", "e", "1/x", "x!", "eˣ", "10ˣ", "Rand", "EE", "|x|", "mod"]
    component Key: Item {
        id: key
        property string label
        readonly property bool op: ["÷", "×", "−", "+", "="].indexOf(label) >= 0
        readonly property bool fn: ["AC", "±", "%", "⌫"].indexOf(label) >= 0
        width: 52; height: 52
        Spring { id: pr; to: ma.pressed ? 1 : 0; preset: Spring.Snappy }
        Rectangle {
            anchors.fill: parent; radius: width / 2
            scale: 1 - 0.06 * pr.value
            color: key.op ? (win.st.pendingOp === key.label && !win.st.fresh ? "white" : "#FF9F0A")
                 : key.fn ? (Theme.dark ? "#5C5C60" : "#D4D4D8") : (Theme.dark ? "#3A3A3C" : "#FFFFFF")
            opacity: 1 - 0.25 * pr.value
            Text { anchors.centerIn: parent; text: key.label; font.family: Theme.fontFamily; font.pixelSize: key.label.length > 2 ? 15 : 22
                   font.weight: Font.Medium; color: key.op ? (win.st.pendingOp === key.label && !win.st.fresh ? "#FF9F0A" : "white") : Theme.label; renderType: Text.QtRendering }
        }
        MouseArea { id: ma; anchors.fill: parent; onClicked: win.press(key.label) }
    }
    Row {
        x: 12; y: 124; spacing: 14
        Grid { visible: win.scientific; columns: 4; spacing: 8
            Repeater { model: win.sci; Key { required property string modelData; label: modelData } } }
        Grid { columns: 4; spacing: 8
            Repeater { model: win.basic; Key { required property string modelData; label: modelData } } }
    }
    Item {
        focus: true
        Keys.onPressed: e => {
            const map = { "*": "×", "/": "÷", "-": "−", "+": "+", "=": "=", "%": "%", ".": ".", ",": ".", "(": "(", ")": ")", "^": "xʸ" }
            if (e.text >= "0" && e.text <= "9") win.press(e.text)
            else if (map[e.text]) win.press(map[e.text])
            else if (e.key === Qt.Key_Return || e.key === Qt.Key_Enter) win.press("=")
            else if (e.key === Qt.Key_Backspace) win.press("⌫")
            else if (e.key === Qt.Key_Escape || e.key === Qt.Key_Delete) win.press("AC")
            e.accepted = true
        }
    }
}
