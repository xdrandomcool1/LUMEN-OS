// SPDX-License-Identifier: GPL-3.0-or-later
// Propagates LUMEN's appearance to third-party toolkits.
#pragma once
#include <QObject>

namespace Lumen {
/*!
 * Writes the LUMEN colour scheme, icon theme and accent into kdeglobals and
 * broadcasts KGlobalSettings::notifyChange so running Qt/KDE apps restyle.
 * xdg-desktop-portal-kde derives org.freedesktop.appearance.color-scheme
 * from the same file, so GTK4/libadwaita and Flatpak apps follow too.
 */
class ThemeSync : public QObject
{
    Q_OBJECT
public:
    explicit ThemeSync(QObject *parent = nullptr);
    void apply();
    static void writeKdeGlobals(bool dark, const QColor &accent);
};
}
