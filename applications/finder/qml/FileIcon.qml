// SPDX-License-Identifier: GPL-3.0-or-later
// File icon: image thumbnail for pictures, themed icon otherwise.
import QtQuick
import org.kde.kirigami as Kirigami

Item {
    id: fi
    property string iconName
    property url url
    property bool isImage: false
    property real size: 64
    width: size; height: size
    Kirigami.Icon {
        anchors.fill: parent
        visible: !thumb.visible
        source: fi.iconName
        roundToIconSize: false
    }
    Image {
        id: thumb
        visible: fi.isImage && status === Image.Ready
        anchors.centerIn: parent
        width: Math.min(fi.size, implicitWidth * fi.size / Math.max(implicitWidth, implicitHeight))
        height: Math.min(fi.size, implicitHeight * fi.size / Math.max(implicitWidth, implicitHeight))
        source: fi.isImage ? fi.url : ""
        sourceSize: Qt.size(fi.size * 2, fi.size * 2)
        asynchronous: true
        fillMode: Image.PreserveAspectFit
        Rectangle { anchors.fill: parent; color: "transparent"; border.width: 2; border.color: "white"; visible: parent.visible }
    }
}
