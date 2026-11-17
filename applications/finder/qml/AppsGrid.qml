// SPDX-License-Identifier: GPL-3.0-or-later
// "Applications" location: installed apps from XDG desktop entries.
import QtQuick
import Lumen.Design
import Lumen.Controls
import Lumen.System
import org.kde.kirigami as Kirigami

GridView {
    anchors.margins: 12
    clip: true
    model: AppsModel
    cellWidth: Math.floor(width / Math.max(1, Math.floor(width / 108)))
    cellHeight: 112
    ScrollBar.vertical: ScrollBar {}
    delegate: Item {
        required property string name
        required property string iconName
        required property string desktopId
        width: GridView.view.cellWidth; height: 112
        Kirigami.Icon { anchors.horizontalCenter: parent.horizontalCenter; y: 6; width: 64; height: 64; source: iconName }
        LText { anchors.horizontalCenter: parent.horizontalCenter; y: 82; width: parent.width - 10; text: name; style: LText.Callout; horizontalAlignment: Text.AlignHCenter; maximumLineCount: 2; wrapMode: Text.Wrap }
        MouseArea { anchors.fill: parent; onDoubleClicked: AppsModel.launch(desktopId) }
    }
}
