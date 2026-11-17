// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Templates as T
import Lumen.Design

T.MenuSeparator {
    implicitWidth: 160
    implicitHeight: 11
    leftPadding: 10; rightPadding: 10
    contentItem: Rectangle { implicitHeight: 1; color: Theme.separator }
}
