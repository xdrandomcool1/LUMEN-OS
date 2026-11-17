// SPDX-License-Identifier: GPL-3.0-or-later
#include "themesync.h"
#include "appearance.h"

#include <QColor>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>

namespace Lumen {
ThemeSync::ThemeSync(QObject *parent) : QObject(parent)
{
    connect(Appearance::instance(), &Appearance::changed, this, &ThemeSync::apply);
    apply();
}

void ThemeSync::writeKdeGlobals(bool dark, const QColor &accent)
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/kdeglobals");
    QSettings s(path, QSettings::IniFormat);
    const QString scheme = dark ? QStringLiteral("LumenDark") : QStringLiteral("LumenLight");
    if (s.value(QStringLiteral("General/ColorScheme")).toString() == scheme
        && s.value(QStringLiteral("General/AccentColor")).toString() == accent.name())
        return;
    s.setValue(QStringLiteral("General/ColorScheme"), scheme);
    s.setValue(QStringLiteral("General/AccentColor"), QStringLiteral("%1,%2,%3").arg(accent.red()).arg(accent.green()).arg(accent.blue()));
    s.setValue(QStringLiteral("Icons/Theme"), QStringLiteral("Lumen"));
    s.setValue(QStringLiteral("KDE/LookAndFeelPackage"), QStringLiteral("org.lumen.desktop"));
    s.sync();
    // KGlobalSettings::PaletteChanged = 0, StyleChanged = 2
    for (int type : {0, 2}) {
        QDBusMessage m = QDBusMessage::createSignal(QStringLiteral("/KGlobalSettings"), QStringLiteral("org.kde.KGlobalSettings"), QStringLiteral("notifyChange"));
        m << type << 0;
        QDBusConnection::sessionBus().send(m);
    }
}

void ThemeSync::apply()
{
    auto *a = Appearance::instance();
    writeKdeGlobals(a->dark(), a->accent());
}
}
