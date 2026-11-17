// SPDX-License-Identifier: GPL-3.0-or-later
// Optional: only loads when the KSyntaxHighlighting QML module is installed.
import QtQuick
import org.kde.syntaxhighlighting
import Lumen.Design

QtObject {
    property var target
    property SyntaxHighlighter hl: SyntaxHighlighter {
        textEdit: target
        theme: Repository.defaultTheme(Theme.dark ? Repository.DarkTheme : Repository.LightTheme)
    }
    function setFile(u) { hl.definition = Repository.definitionForFileName(u.toString()) }
}
