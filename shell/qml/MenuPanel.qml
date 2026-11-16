// SPDX-License-Identifier: GPL-3.0-or-later
// Drop-down menu panel (menu bar menus, Dock context menus), with
// cascading submenus and keyboard navigation.
import QtQuick
import Lumen.Design
import Lumen.Motion
import Lumen.Glass
import Lumen.Controls
import Lumen.Shell

GlassSurface {
    id: panel
    property var items: []
    property int currentIndex: -1
    signal done()

    readonly property real itemH: Theme.metrics.menuItemHeight
    width: Math.max(220, measure.maxWidth + 60)
    height: col.implicitHeight + 10
    radius: Theme.metrics.menuRadius
    material: Glass.Thin
    shadowScale: 0.9
    shadowOffsetY: 8

    Item {
        id: measure
        visible: false
        property real maxWidth: {
            let w = 0
            for (let i = 0; i < panel.items.length; ++i) {
                const it = panel.items[i]
                if (it.separator) continue
                metrics.text = (it.text || "") + "      " + (it.shortcut || "")
                w = Math.max(w, metrics.advanceWidth)
            }
            return w
        }
        TextMetrics { id: metrics; font.family: Theme.fontFamily; font.pixelSize: 13 }
    }

    function activate(i) {
        const it = items[i]
        if (!it || it.separator || it.enabled === false) return
        if (it.children && it.children.length) return
        if (it.action) it.action()
        else if (it.path !== undefined) AppMenu.trigger(it.path)
        panel.done()
    }
    function move(d) {
        let i = currentIndex
        for (let n = 0; n < items.length; ++n) {
            i = (i + d + items.length) % items.length
            if (!items[i].separator && items[i].enabled !== false) { currentIndex = i; return }
        }
    }

    Column {
        id: col
        x: 5; y: 5
        width: panel.width - 10
        Repeater {
            model: panel.items
            delegate: Item {
                id: row
                required property var modelData
                required property int index
                readonly property bool isSep: modelData.separator === true
                readonly property bool enabledItem: modelData.enabled !== false
                readonly property bool hi: panel.currentIndex === index && enabledItem && !isSep
                width: col.width
                height: isSep ? 11 : panel.itemH
                Rectangle { visible: row.isSep; anchors.verticalCenter: parent.verticalCenter; x: 10; width: parent.width - 20; height: 1; color: Theme.separator }
                Rectangle { visible: row.hi; anchors.fill: parent; radius: Theme.metrics.menuItemRadius; color: Theme.menuHighlight }
                Symbol { visible: !row.isSep && row.modelData.checked === true; x: 6; anchors.verticalCenter: parent.verticalCenter
                         width: 12; height: 12; name: "check"; weight: Symbol.Bold; color: row.hi ? "white" : Theme.label }
                Text {
                    visible: !row.isSep
                    x: 22; anchors.verticalCenter: parent.verticalCenter
                    text: row.modelData.text || ""
                    font.family: Theme.fontFamily; font.pixelSize: 13
                    color: !row.enabledItem ? Theme.tertiaryLabel : row.hi ? "white" : Theme.label
                    renderType: Text.QtRendering
                }
                Text {
                    visible: !row.isSep && (row.modelData.shortcut || "").length > 0 && !row.modelData.children
                    anchors.right: parent.right; anchors.rightMargin: 10; anchors.verticalCenter: parent.verticalCenter
                    text: row.modelData.shortcut || ""
                    font.family: Theme.fontFamily; font.pixelSize: 13
                    color: row.hi ? Qt.rgba(1, 1, 1, 0.8) : Theme.tertiaryLabel
                    renderType: Text.QtRendering
                }
                Symbol {
                    visible: !row.isSep && row.modelData.children !== undefined && row.modelData.children.length > 0
                    anchors.right: parent.right; anchors.rightMargin: 8; anchors.verticalCenter: parent.verticalCenter
                    width: 11; height: 11; name: "caret-right"; weight: Symbol.Bold; color: row.hi ? "white" : Theme.label
                }
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    enabled: !row.isSep
                    onEntered: { panel.currentIndex = row.index; subTimer.restart() }
                    onClicked: panel.activate(row.index)
                }
            }
        }
    }

    // Cascading submenu
    Timer {
        id: subTimer
        interval: 180
        onTriggered: {
            const it = panel.items[panel.currentIndex]
            if (it && it.children && it.children.length) {
                sub.subItems = it.children
                sub.y = panel.currentIndex * panel.itemH
                sub.active = true
            } else {
                sub.active = false
            }
        }
    }
    Loader {
        id: sub
        property var subItems: []
        active: false
        x: panel.width - 6
        source: "MenuPanel.qml"
        onLoaded: {
            item.items = Qt.binding(() => sub.subItems)
            item.backdrop = panel.backdrop
            item.compositorBlur = panel.compositorBlur
            item.done.connect(panel.done)
        }
    }
}
