// SPDX-License-Identifier: GPL-3.0-or-later
#include "shellsettings.h"
#include <QDir>
#include <QFileInfo>
#include <QQmlEngine>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>

namespace Lumen {

static QString userPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/lumen/lumenrc");
}

ShellSettings::ShellSettings(QObject *parent)
    : QObject(parent)
    , m_dockIconSize(52)
    , m_dockMagnification(true)
    , m_dockMagnifiedSize(84)
    , m_dockPosition(QStringLiteral("bottom"))
    , m_dockAutoHide(false)
    , m_dockShowRecents(false)
    , m_pinnedApps({QStringLiteral("applications:lumen-finder.desktop"), QStringLiteral("applications:org.mozilla.firefox.desktop"),
                    QStringLiteral("applications:lumen-terminal.desktop"), QStringLiteral("applications:lumen-store.desktop"),
                    QStringLiteral("applications:lumen-calendar.desktop"), QStringLiteral("applications:lumen-texteditor.desktop"),
                    QStringLiteral("applications:lumen-activity-monitor.desktop"), QStringLiteral("applications:lumen-settings.desktop")})
    , m_menuBarBackground(false)
    , m_showBatteryPercentage(true)
    , m_clock24h(true)
    , m_dynamicWallpaper(QStringLiteral("lumen"))
    , m_weatherLatitude(0)
    , m_weatherLongitude(0)
    , m_desktopWidgets({QStringLiteral("clock"), QStringLiteral("calendar"), QStringLiteral("weather"), QStringLiteral("system")})
{
    load();
    QDir().mkpath(QFileInfo(userPath()).absolutePath());
    m_watcher.addPath(QFileInfo(userPath()).absolutePath());
    if (QFileInfo::exists(userPath()))
        m_watcher.addPath(userPath());
    auto reload = [this] {
        if (m_saving) return;
        load();
        if (QFileInfo::exists(userPath()) && !m_watcher.files().contains(userPath()))
            m_watcher.addPath(userPath());
        Q_EMIT changed();
    };
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, reload);
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, reload);
}

ShellSettings *ShellSettings::instance()
{
    static ShellSettings *s = new ShellSettings;
    return s;
}

ShellSettings *ShellSettings::create(QQmlEngine *, QJSEngine *)
{
    QQmlEngine::setObjectOwnership(instance(), QQmlEngine::CppOwnership);
    return instance();
}

void ShellSettings::load()
{
    const QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation);
    for (auto it = dirs.crbegin(); it != dirs.crend(); ++it) {
        const QString path = *it + QStringLiteral("/lumen/lumenrc");
        if (!QFileInfo::exists(path))
            continue;
        QSettings s(path, QSettings::IniFormat);
        m_dockIconSize = qBound(24, s.value(QStringLiteral("Dock/iconSize"), m_dockIconSize).toInt(), 128);
        m_dockMagnification = s.value(QStringLiteral("Dock/magnification"), m_dockMagnification).toBool();
        m_dockMagnifiedSize = qBound(m_dockIconSize, s.value(QStringLiteral("Dock/magnifiedSize"), m_dockMagnifiedSize).toInt(), 160);
        m_dockPosition = s.value(QStringLiteral("Dock/position"), m_dockPosition).toString();
        m_dockAutoHide = s.value(QStringLiteral("Dock/autoHide"), m_dockAutoHide).toBool();
        m_dockShowRecents = s.value(QStringLiteral("Dock/showRecents"), m_dockShowRecents).toBool();
        m_pinnedApps = s.value(QStringLiteral("Dock/pinned"), m_pinnedApps).toStringList();
        m_menuBarBackground = s.value(QStringLiteral("MenuBar/background"), m_menuBarBackground).toBool();
        m_showBatteryPercentage = s.value(QStringLiteral("MenuBar/batteryPercentage"), m_showBatteryPercentage).toBool();
        m_clock24h = s.value(QStringLiteral("MenuBar/clock24h"), m_clock24h).toBool();
        m_wallpaper = s.value(QStringLiteral("Wallpaper/image"), m_wallpaper).toString();
        m_dynamicWallpaper = s.value(QStringLiteral("Wallpaper/dynamic"), m_dynamicWallpaper).toString();
        m_weatherCity = s.value(QStringLiteral("Weather/city"), m_weatherCity).toString();
        m_weatherLatitude = s.value(QStringLiteral("Weather/latitude"), m_weatherLatitude).toDouble();
        m_weatherLongitude = s.value(QStringLiteral("Weather/longitude"), m_weatherLongitude).toDouble();
        m_desktopWidgets = s.value(QStringLiteral("Desktop/widgets"), m_desktopWidgets).toStringList();
    }
}

void ShellSettings::save()
{
    m_saving = true;
    {
        QSettings s(userPath(), QSettings::IniFormat);
        s.setValue(QStringLiteral("Dock/iconSize"), m_dockIconSize);
        s.setValue(QStringLiteral("Dock/magnification"), m_dockMagnification);
        s.setValue(QStringLiteral("Dock/magnifiedSize"), m_dockMagnifiedSize);
        s.setValue(QStringLiteral("Dock/position"), m_dockPosition);
        s.setValue(QStringLiteral("Dock/autoHide"), m_dockAutoHide);
        s.setValue(QStringLiteral("Dock/showRecents"), m_dockShowRecents);
        s.setValue(QStringLiteral("Dock/pinned"), m_pinnedApps);
        s.setValue(QStringLiteral("MenuBar/background"), m_menuBarBackground);
        s.setValue(QStringLiteral("MenuBar/batteryPercentage"), m_showBatteryPercentage);
        s.setValue(QStringLiteral("MenuBar/clock24h"), m_clock24h);
        s.setValue(QStringLiteral("Wallpaper/image"), m_wallpaper);
        s.setValue(QStringLiteral("Wallpaper/dynamic"), m_dynamicWallpaper);
        s.setValue(QStringLiteral("Weather/city"), m_weatherCity);
        s.setValue(QStringLiteral("Weather/latitude"), m_weatherLatitude);
        s.setValue(QStringLiteral("Weather/longitude"), m_weatherLongitude);
        s.setValue(QStringLiteral("Desktop/widgets"), m_desktopWidgets);
    }
    QTimer::singleShot(200, this, [this] { m_saving = false; });
}

void ShellSettings::pin(const QString &url)
{
    if (m_pinnedApps.contains(url)) return;
    auto l = m_pinnedApps; l << url; setPinnedApps(l);
}

void ShellSettings::unpin(const QString &url)
{
    auto l = m_pinnedApps; l.removeAll(url); setPinnedApps(l);
}

bool ShellSettings::isPinned(const QString &url) const { return m_pinnedApps.contains(url); }
}
