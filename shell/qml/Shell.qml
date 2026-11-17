// SPDX-License-Identifier: GPL-3.0-or-later
// Root of lumen-shell: one set of surfaces per screen.
import QtQuick
import Lumen.Shell
import Lumen.System

QtObject {
    id: shell

    property Instantiator screens: Instantiator {
        model: Qt.application.screens
        delegate: ScreenShell {
            required property var modelData
            required property int index
            screen: modelData
            primary: index === 0
        }
    }
}
