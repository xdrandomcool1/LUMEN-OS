// SPDX-License-Identifier: GPL-3.0-or-later
// All shell surfaces of one screen.
import QtQuick
import Lumen.Shell
import Lumen.System

QtObject {
    id: s
    property var screen
    property bool primary: false

    property Desktop desktop: Desktop { targetScreen: s.screen }
    property MenuBar menuBar: MenuBar { targetScreen: s.screen }
    property Dock dock: primary ? dockComponent.createObject(s, { targetScreen: s.screen }) : null
    property PanelHost panels: PanelHost { targetScreen: s.screen; primary: s.primary }
    property NotificationBanners banners: primary ? bannersComponent.createObject(s, { targetScreen: s.screen }) : null

    property Component dockComponent: Component { Dock {} }
    property Component bannersComponent: Component { NotificationBanners {} }
}
