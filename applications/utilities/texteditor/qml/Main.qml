// SPDX-License-Identifier: GPL-3.0-or-later
// Text Editor: open/save, find & replace, syntax highlighting, word count.
import QtQuick
import QtQuick.Dialogs
import QtCore
import Lumen.Design
import Lumen.Controls
import Lumen.TextEditor

AppWindow {
    id: win
    property string openFile
    property url fileUrl
    property bool dirty: false
    title: (fileUrl.toString().length ? fileUrl.toString().split("/").pop() : qsTr("Untitled")) + (dirty ? " — " + qsTr("Edited") : "")
    subtitle: doc.lines + qsTr(" lines · ") + doc.words + qsTr(" words")
    width: 860; height: 620
    visible: true
    contentColor: Theme.contentBackground
    Settings { id: prefs; category: "TextEditor"; property real fontSize: 14; property bool wrap: true; property bool mono: true }

    Document { id: doc; text: editor.text }
    function load(u) { const r = doc.load(u); if (r.ok) { editor.text = r.text; fileUrl = u; dirty = false; highlighter.setFile(u) } else status.text = r.error }
    function save(u) { const err = doc.save(u, editor.text); if (!err) { fileUrl = u; dirty = false; status.text = qsTr("Saved") } else status.text = err }
    Component.onCompleted: if (openFile.length) load(openFile)

    toolbarTrailing: [
        ToolbarGroup {
            ToolbarButton { symbol: "folder-open"; tooltip: qsTr("Open…"); onClicked: openDlg.open() }
            ToolbarButton { symbol: "floppy-disk"; tooltip: qsTr("Save"); onClicked: win.fileUrl.toString().length ? win.save(win.fileUrl) : saveDlg.open() }
        },
        ToolbarGroup {
            ToolbarButton { symbol: "magnifying-glass"; tooltip: qsTr("Find"); checked: findBar.visible; onClicked: { findBar.visible = !findBar.visible; if (findBar.visible) findField.forceActiveFocus() } }
            ToolbarButton { symbol: "text-aa"; tooltip: qsTr("Monospace"); checked: prefs.mono; onClicked: prefs.mono = !prefs.mono }
            ToolbarButton { symbol: "arrows-left-right"; tooltip: qsTr("Wrap lines"); checked: prefs.wrap; onClicked: prefs.wrap = !prefs.wrap }
        }
    ]
    FileDialog { id: openDlg; onAccepted: win.load(selectedFile) }
    FileDialog { id: saveDlg; fileMode: FileDialog.SaveFile; onAccepted: win.save(selectedFile) }

    Row {
        id: findBar
        visible: false
        x: 12; y: win.toolbarHeight + 2; spacing: 8; height: visible ? 32 : 0
        SearchField { id: findField; width: 220; onAccepted: win.findNext() }
        TextField { id: replaceField; width: 180; placeholderText: qsTr("Replace") }
        Button { text: qsTr("Next"); onClicked: win.findNext() }
        Button { text: qsTr("Replace All"); onClicked: { const n = doc.count(editor.text, findField.text); editor.text = editor.text.split(findField.text).join(replaceField.text); status.text = qsTr("%n replacement(s)", "", n) } }
    }
    function findNext() {
        const q = findField.text; if (!q.length) return
        let i = editor.text.indexOf(q, editor.cursorPosition)
        if (i < 0) i = editor.text.indexOf(q)
        if (i >= 0) { editor.select(i, i + q.length); editor.forceActiveFocus() } else status.text = qsTr("Not found")
    }

    Flickable {
        id: flick
        x: 0; y: win.toolbarHeight + findBar.height + 4
        width: win.contentArea.width; height: win.contentArea.height - y - 22
        contentWidth: prefs.wrap ? width : editor.implicitWidth; contentHeight: editor.implicitHeight
        clip: true
        ScrollBar.vertical: ScrollBar {}
        ScrollBar.horizontal: ScrollBar {}
        TextEdit {
            id: editor
            width: prefs.wrap ? flick.width : Math.max(flick.width, implicitWidth)
            leftPadding: 20; rightPadding: 20; topPadding: 10; bottomPadding: 40
            wrapMode: prefs.wrap ? TextEdit.Wrap : TextEdit.NoWrap
            font.family: prefs.mono ? Theme.monoFamily : Theme.fontFamily
            font.pixelSize: prefs.fontSize
            color: Theme.label
            selectionColor: Theme.selection; selectedTextColor: "white"
            selectByMouse: true; persistentSelection: true
            textFormat: TextEdit.PlainText
            focus: true
            onTextChanged: win.dirty = true
            onCursorRectangleChanged: {
                if (cursorRectangle.y < flick.contentY) flick.contentY = cursorRectangle.y
                else if (cursorRectangle.y + cursorRectangle.height > flick.contentY + flick.height) flick.contentY = cursorRectangle.y + cursorRectangle.height - flick.height
            }
        }
    }
    // Syntax highlighting via KSyntaxHighlighting when installed.
    Loader {
        id: highlighter
        function setFile(u) { if (item) item.setFile(u) }
        source: "Highlighter.qml"
        onLoaded: { item.target = editor; if (win.fileUrl.toString().length) item.setFile(win.fileUrl) }
    }
    LText { id: status; anchors.bottom: parent.bottom; anchors.right: parent.right; anchors.margins: 6; style: LText.Caption; tertiary: true }
    Shortcut { sequences: [StandardKey.Open]; onActivated: openDlg.open() }
    Shortcut { sequences: [StandardKey.Save]; onActivated: win.fileUrl.toString().length ? win.save(win.fileUrl) : saveDlg.open() }
    Shortcut { sequences: [StandardKey.SaveAs]; onActivated: saveDlg.open() }
    Shortcut { sequences: [StandardKey.Find]; onActivated: { findBar.visible = true; findField.forceActiveFocus() } }
    Shortcut { sequences: [StandardKey.ZoomIn]; onActivated: prefs.fontSize = Math.min(40, prefs.fontSize + 1) }
    Shortcut { sequences: [StandardKey.ZoomOut]; onActivated: prefs.fontSize = Math.max(8, prefs.fontSize - 1) }
}
