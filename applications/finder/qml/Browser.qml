// SPDX-License-Identifier: GPL-3.0-or-later
// One Finder tab: navigation history, icon/list views, selection, file
// operations, drag & drop, Quick Look and Get Info.
import QtQuick
import Lumen.Design
import Lumen.Motion
import Lumen.Glass
import Lumen.Controls
import Lumen.System
import Lumen.Finder

FocusScope {
    id: br
    property string initialUrl
    property int viewMode: 0      // 0 icons, 1 list
    property bool showHidden: false
    signal openInNewTab(url url)
    signal toggleHidden()

    readonly property alias model: folder
    readonly property bool isApps: folder.url.toString() === "lumen-applications:/"
    property var history: []
    property int historyPos: -1
    readonly property bool canBack: historyPos > 0
    readonly property bool canForward: historyPos < history.length - 1

    property var selection: ({})       // row -> true
    property int anchorRow: -1
    property int renameRow: -1
    readonly property var selectedRows: Object.keys(selection).map(Number).filter(r => selection[r])
    readonly property int selectedCount: selectedRows.length

    FolderModel {
        id: folder
        showHidden: br.showHidden
        onNavigateRequested: u => br.navigate(u)
        onErrorOccurred: msg => toast.show(msg)
        onUrlChanged: br.clearSelection()
    }

    function navigate(url, fromHistory) {
        const u = url.toString()
        if (!fromHistory) {
            history = history.slice(0, historyPos + 1).concat([u])
            historyPos = history.length - 1
        }
        folder.url = u
        view.forceActiveFocus()
    }
    function back() { if (canBack) { historyPos--; navigate(history[historyPos], true) } }
    function forward() { if (canForward) { historyPos++; navigate(history[historyPos], true) } }
    Component.onCompleted: navigate(initialUrl)

    function clearSelection() { selection = ({}); anchorRow = -1; renameRow = -1 }
    function select(row, mods) {
        const s = Object.assign({}, selection)
        if (mods & Qt.ShiftModifier && anchorRow >= 0) {
            const a = Math.min(anchorRow, row), b = Math.max(anchorRow, row)
            for (let r = a; r <= b; ++r) s[r] = true
        } else if (mods & Qt.ControlModifier) {
            s[row] = !s[row]; anchorRow = row
        } else {
            for (const k in s) delete s[k]
            s[row] = true; anchorRow = row
        }
        selection = s
        currentView().currentIndex = row
    }
    function selectAll() { const s = {}; for (let r = 0; r < folder.count; ++r) s[r] = true; selection = s }
    function currentView() { return br.viewMode === 0 ? grid : list }
    function newFolder() { folder.createFolder(qsTr("untitled folder")) }
    function rows() { return selectedCount ? selectedRows : (currentView().currentIndex >= 0 ? [currentView().currentIndex] : []) }

    // ---- Views -------------------------------------------------------------
    Item {
        id: view
        anchors.fill: parent
        focus: true
        visible: !br.isApps

        Keys.onPressed: e => {
            const ctrl = e.modifiers & Qt.ControlModifier
            if (e.key === Qt.Key_Space) { if (br.rows().length) quickLook.openFor(br.rows()[0]); e.accepted = true }
            else if (e.key === Qt.Key_Return || e.key === Qt.Key_Enter || e.key === Qt.Key_F2) { if (br.rows().length === 1) br.renameRow = br.rows()[0]; e.accepted = true }
            else if ((ctrl && e.key === Qt.Key_O) || (ctrl && e.key === Qt.Key_Down)) { br.rows().forEach(r => folder.open(r)); e.accepted = true }
            else if (ctrl && e.key === Qt.Key_Up) { if (folder.canGoUp) br.navigate(folder.parentUrl()); e.accepted = true }
            else if (e.key === Qt.Key_Delete || (ctrl && e.key === Qt.Key_Backspace)) { folder.trash(br.rows()); br.clearSelection(); e.accepted = true }
            else if (ctrl && e.key === Qt.Key_A) { br.selectAll(); e.accepted = true }
            else if (ctrl && e.key === Qt.Key_C) { folder.copyToClipboard(br.rows(), false); e.accepted = true }
            else if (ctrl && e.key === Qt.Key_X) { folder.copyToClipboard(br.rows(), true); e.accepted = true }
            else if (ctrl && e.key === Qt.Key_V) { folder.paste(); e.accepted = true }
            else if (ctrl && e.key === Qt.Key_D) { folder.duplicate(br.rows()); e.accepted = true }
            else if (ctrl && e.key === Qt.Key_I) { if (br.rows().length) info.openFor(br.rows()[0]); e.accepted = true }
            else if (ctrl && (e.modifiers & Qt.ShiftModifier) && e.key === Qt.Key_N) { br.newFolder(); e.accepted = true }
            else if (e.key === Qt.Key_Escape) { br.clearSelection(); e.accepted = true }
        }

        GridView {
            id: grid
            anchors.fill: parent
            anchors.margins: 12
            visible: br.viewMode === 0
            model: folder
            cellWidth: Math.floor(width / Math.max(1, Math.floor(width / 108)))
            cellHeight: 112
            clip: true
            keyNavigationEnabled: true
            boundsBehavior: Flickable.DragAndOvershootBounds
            ScrollBar.vertical: ScrollBar {}
            onCurrentIndexChanged: if (activeFocus && currentIndex >= 0) br.select(currentIndex, 0)
            delegate: FileTile { browser: br; folder: br.model; viewWidth: grid.cellWidth }
            add: Transition { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Motion.fast } }
            remove: Transition { NumberAnimation { property: "opacity"; to: 0; duration: Motion.fast } }
        }

        ListView {
            id: list
            anchors.fill: parent
            anchors.topMargin: 6
            visible: br.viewMode === 1
            model: folder
            clip: true
            keyNavigationEnabled: true
            ScrollBar.vertical: ScrollBar {}
            headerPositioning: ListView.OverlayHeader
            header: ListHeader { width: list.width; folder: br.model; z: 2 }
            onCurrentIndexChanged: if (activeFocus && currentIndex >= 0) br.select(currentIndex, 0)
            delegate: FileRow { browser: br; folder: br.model; width: list.width }
        }

        // Background: deselect, context menu, drop target.
        MouseArea {
            anchors.fill: parent
            z: -1
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onClicked: m => { br.clearSelection(); view.forceActiveFocus(); if (m.button === Qt.RightButton) br.showBackgroundMenu() }
        }
        DropArea {
            anchors.fill: parent
            z: -2
            keys: ["text/uri-list"]
            onDropped: d => { folder.copyUrls(d.urls, folder.url, !(d.modifiers & Qt.ControlModifier) && folder.url.toString().startsWith("file:")); d.acceptProposedAction() }
        }

        Column {
            anchors.centerIn: parent
            visible: folder.count === 0 && !folder.busy
            spacing: 6
            Symbol { anchors.horizontalCenter: parent.horizontalCenter; width: 40; height: 40; name: folder.isTrash ? "trash" : "folder-open"; color: Theme.tertiaryLabel }
            LText { anchors.horizontalCenter: parent.horizontalCenter; text: folder.filterText.length ? qsTr("No Results") : folder.isTrash ? qsTr("Trash is Empty") : qsTr("Folder is Empty"); tertiary: true; style: LText.Title3 }
        }
        Spinner { anchors.centerIn: parent; running: folder.busy && folder.count === 0 }

        LText {
            anchors.right: parent.right; anchors.bottom: parent.bottom; anchors.margins: 10
            text: br.selectedCount > 0 ? qsTr("%1 of %2 selected").arg(br.selectedCount).arg(folder.count) : qsTr("%n items", "", folder.count)
            style: LText.Caption; tertiary: true
        }
    }

    AppsGrid { anchors.fill: parent; visible: br.isApps }

    // ---- Context menus ----------------------------------------------------
    Menu {
        id: itemMenu
        property int row: -1
        MenuItem { text: qsTr("Open"); onTriggered: br.rows().forEach(r => folder.open(r)) }
        MenuItem { text: qsTr("Open in New Tab"); visible: folder.isDirAt(itemMenu.row); height: visible ? implicitHeight : 0; onTriggered: br.openInNewTab(folder.urlAt(itemMenu.row)) }
        Menu {
            id: openWith
            title: qsTr("Open With")
            Instantiator {
                model: itemMenu.row >= 0 ? folder.openWithApps(itemMenu.row) : []
                delegate: MenuItem { required property var modelData; text: modelData.name; onTriggered: folder.openWithApp(itemMenu.row, modelData.desktopId) }
                onObjectAdded: (i, o) => openWith.insertItem(i, o)
                onObjectRemoved: (i, o) => openWith.removeItem(o)
            }
        }
        MenuSeparator {}
        MenuItem { text: folder.isTrash ? qsTr("Put Back") : qsTr("Move to Trash"); onTriggered: { if (folder.isTrash) folder.restore(br.rows()); else folder.trash(br.rows()); br.clearSelection() } }
        MenuItem { visible: folder.isTrash; height: visible ? implicitHeight : 0; text: qsTr("Delete Immediately…"); destructive: true; onTriggered: { folder.deletePermanently(br.rows()); br.clearSelection() } }
        MenuSeparator {}
        MenuItem { text: qsTr("Get Info"); shortcutText: "Ctrl+I"; onTriggered: info.openFor(itemMenu.row) }
        MenuItem { text: qsTr("Rename"); enabled: br.rows().length === 1; onTriggered: br.renameRow = itemMenu.row }
        MenuItem { text: qsTr("Compress"); onTriggered: folder.compress(br.rows()) }
        MenuItem { text: qsTr("Duplicate"); shortcutText: "Ctrl+D"; onTriggered: folder.duplicate(br.rows()) }
        MenuItem { text: qsTr("Quick Look"); shortcutText: "Space"; onTriggered: quickLook.openFor(itemMenu.row) }
        MenuSeparator {}
        MenuItem { text: qsTr("Copy"); shortcutText: "Ctrl+C"; onTriggered: folder.copyToClipboard(br.rows(), false) }
    }
    Menu {
        id: bgMenu
        MenuItem { text: qsTr("New Folder"); shortcutText: "Ctrl+Shift+N"; enabled: !folder.isTrash; onTriggered: br.newFolder() }
        MenuItem { text: qsTr("Paste"); shortcutText: "Ctrl+V"; enabled: folder.canPaste() && !folder.isTrash; onTriggered: folder.paste() }
        MenuSeparator {}
        MenuItem { text: br.showHidden ? qsTr("Hide Hidden Files") : qsTr("Show Hidden Files"); onTriggered: br.toggleHidden() }
        Menu {
            id: sortMenu
            title: qsTr("Sort By")
            Instantiator {
                model: [["name", qsTr("Name")], ["modified", qsTr("Date Modified")], ["size", qsTr("Size")], ["kind", qsTr("Kind")]]
                delegate: MenuItem { required property var modelData; text: modelData[1]; checkable: true; checked: folder.sortKey === modelData[0]; onTriggered: folder.sortKey = modelData[0] }
                onObjectAdded: (i, o) => sortMenu.insertItem(i, o)
                onObjectRemoved: (i, o) => sortMenu.removeItem(o)
            }
        }
        MenuItem { visible: folder.isTrash; height: visible ? implicitHeight : 0; text: qsTr("Empty Trash"); onTriggered: folder.emptyTrash() }
    }
    function showItemMenu(row) {
        if (!selection[row]) select(row, 0)
        itemMenu.row = row
        itemMenu.popup()
    }
    function showBackgroundMenu() { bgMenu.popup() }

    QuickLook { id: quickLook; folder: br.model }
    InfoPanel { id: info; folder: br.model }

    // Error toast
    Rectangle {
        id: toast
        property string text
        function show(t) { text = t; opacity = 1; toastTimer.restart() }
        anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom; anchors.bottomMargin: 30
        width: toastText.implicitWidth + 30; height: 34; radius: height / 2
        color: Theme.dark ? "#3a3a3c" : "#ffffff"; border.width: 0.5; border.color: Theme.controlBorder
        opacity: 0; visible: opacity > 0
        Behavior on opacity { NumberAnimation { duration: Motion.normal } }
        LText { id: toastText; anchors.centerIn: parent; text: toast.text; color: Theme.red }
        Timer { id: toastTimer; interval: 4000; onTriggered: toast.opacity = 0 }
    }
}
