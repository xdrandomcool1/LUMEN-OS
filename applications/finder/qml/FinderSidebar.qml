// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtCore
import Lumen.Design
import Lumen.Controls
import Lumen.Finder.Kio

Flickable {
    id: sb
    property string currentUrl
    signal open(url url)
    contentHeight: col.implicitHeight + 20
    clip: true
    boundsBehavior: Flickable.StopAtBounds

    function loc(l) { return StandardPaths.writableLocation(l).toString() }
    readonly property var favorites: [
        { name: qsTr("Home"), symbol: "house", url: loc(StandardPaths.HomeLocation) },
        { name: qsTr("Applications"), symbol: "squares-four", url: "lumen-applications:/" },
        { name: qsTr("Desktop"), symbol: "desktop", url: loc(StandardPaths.DesktopLocation) },
        { name: qsTr("Documents"), symbol: "file-text", url: loc(StandardPaths.DocumentsLocation) },
        { name: qsTr("Downloads"), symbol: "download-simple", url: loc(StandardPaths.DownloadLocation) },
        { name: qsTr("Pictures"), symbol: "image", url: loc(StandardPaths.PicturesLocation) },
        { name: qsTr("Music"), symbol: "music-notes", url: loc(StandardPaths.MusicLocation) },
        { name: qsTr("Videos"), symbol: "film-strip", url: loc(StandardPaths.MoviesLocation) }
    ]

    PlacesModel { id: places }

    Column {
        id: col
        x: 8; width: sb.width - 16
        SidebarHeader { text: qsTr("Favorites") }
        Repeater {
            model: sb.favorites
            SidebarItem {
                required property var modelData
                width: col.width
                text: modelData.name; symbol: modelData.symbol
                selected: sb.currentUrl === modelData.url || (sb.currentUrl + "/") === modelData.url
                onClicked: sb.open(modelData.url)
            }
        }
        SidebarHeader { text: qsTr("Locations") }
        SidebarItem { width: col.width; text: qsTr("Computer"); symbol: "laptop"; selected: sb.currentUrl === "file:///"; onClicked: sb.open("file:///") }
        Repeater {
            // Removable / mounted devices from Solid (USB drives, other disks).
            model: places
            SidebarItem {
                required property var model
                required property int index
                visible: model.isDevice === true && model.url && model.url.toString().length > 0
                height: visible ? implicitHeight : 0
                width: col.width
                text: model.display; symbol: model.isRemovable ? "usb" : "hard-drive"
                onClicked: sb.open(model.url)
            }
        }
        SidebarItem { width: col.width; text: qsTr("Trash"); symbol: "trash"; selected: sb.currentUrl.startsWith("trash:"); onClicked: sb.open("trash:/") }
    }
}
