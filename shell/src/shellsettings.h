// SPDX-License-Identifier: GPL-3.0-or-later
// Persistent shell preferences ([Dock], [MenuBar], [Wallpaper] in lumenrc).
#pragma once
#include <QFileSystemWatcher>
#include <QObject>
#include <QStringList>
#include <QQmlEngine>
#include <QtQml/qqmlregistration.h>

namespace Lumen {
class ShellSettings : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(int dockIconSize READ dockIconSize WRITE setDockIconSize NOTIFY changed)
    Q_PROPERTY(bool dockMagnification READ dockMagnification WRITE setDockMagnification NOTIFY changed)
    Q_PROPERTY(int dockMagnifiedSize READ dockMagnifiedSize WRITE setDockMagnifiedSize NOTIFY changed)
    Q_PROPERTY(QString dockPosition READ dockPosition WRITE setDockPosition NOTIFY changed)
    Q_PROPERTY(bool dockAutoHide READ dockAutoHide WRITE setDockAutoHide NOTIFY changed)
    Q_PROPERTY(bool dockShowRecents READ dockShowRecents WRITE setDockShowRecents NOTIFY changed)
    Q_PROPERTY(QStringList pinnedApps READ pinnedApps WRITE setPinnedApps NOTIFY changed)
    Q_PROPERTY(bool menuBarBackground READ menuBarBackground WRITE setMenuBarBackground NOTIFY changed)
    Q_PROPERTY(bool showBatteryPercentage READ showBatteryPercentage WRITE setShowBatteryPercentage NOTIFY changed)
    Q_PROPERTY(bool clock24h READ clock24h WRITE setClock24h NOTIFY changed)
    Q_PROPERTY(QString wallpaper READ wallpaper WRITE setWallpaper NOTIFY changed)
    Q_PROPERTY(QString dynamicWallpaper READ dynamicWallpaper WRITE setDynamicWallpaper NOTIFY changed)

    Q_PROPERTY(QString weatherCity READ weatherCity WRITE setWeatherCity NOTIFY changed)
    Q_PROPERTY(double weatherLatitude READ weatherLatitude WRITE setWeatherLatitude NOTIFY changed)
    Q_PROPERTY(double weatherLongitude READ weatherLongitude WRITE setWeatherLongitude NOTIFY changed)
    Q_PROPERTY(QStringList desktopWidgets READ desktopWidgets WRITE setDesktopWidgets NOTIFY changed)

public:
    QString weatherCity() const { return m_weatherCity; }
    void setWeatherCity(const QString &v) { if (m_weatherCity == v) return; m_weatherCity = v; save(); Q_EMIT changed(); }
    double weatherLatitude() const { return m_weatherLatitude; }
    void setWeatherLatitude(const double &v) { if (m_weatherLatitude == v) return; m_weatherLatitude = v; save(); Q_EMIT changed(); }
    double weatherLongitude() const { return m_weatherLongitude; }
    void setWeatherLongitude(const double &v) { if (m_weatherLongitude == v) return; m_weatherLongitude = v; save(); Q_EMIT changed(); }
    QStringList desktopWidgets() const { return m_desktopWidgets; }
    void setDesktopWidgets(const QStringList &v) { if (m_desktopWidgets == v) return; m_desktopWidgets = v; save(); Q_EMIT changed(); }
    int dockIconSize() const { return m_dockIconSize; }
    void setDockIconSize(const int &v) { if (m_dockIconSize == v) return; m_dockIconSize = v; save(); Q_EMIT changed(); }
    bool dockMagnification() const { return m_dockMagnification; }
    void setDockMagnification(const bool &v) { if (m_dockMagnification == v) return; m_dockMagnification = v; save(); Q_EMIT changed(); }
    int dockMagnifiedSize() const { return m_dockMagnifiedSize; }
    void setDockMagnifiedSize(const int &v) { if (m_dockMagnifiedSize == v) return; m_dockMagnifiedSize = v; save(); Q_EMIT changed(); }
    QString dockPosition() const { return m_dockPosition; }
    void setDockPosition(const QString &v) { if (m_dockPosition == v) return; m_dockPosition = v; save(); Q_EMIT changed(); }
    bool dockAutoHide() const { return m_dockAutoHide; }
    void setDockAutoHide(const bool &v) { if (m_dockAutoHide == v) return; m_dockAutoHide = v; save(); Q_EMIT changed(); }
    bool dockShowRecents() const { return m_dockShowRecents; }
    void setDockShowRecents(const bool &v) { if (m_dockShowRecents == v) return; m_dockShowRecents = v; save(); Q_EMIT changed(); }
    QStringList pinnedApps() const { return m_pinnedApps; }
    void setPinnedApps(const QStringList &v) { if (m_pinnedApps == v) return; m_pinnedApps = v; save(); Q_EMIT changed(); }
    bool menuBarBackground() const { return m_menuBarBackground; }
    void setMenuBarBackground(const bool &v) { if (m_menuBarBackground == v) return; m_menuBarBackground = v; save(); Q_EMIT changed(); }
    bool showBatteryPercentage() const { return m_showBatteryPercentage; }
    void setShowBatteryPercentage(const bool &v) { if (m_showBatteryPercentage == v) return; m_showBatteryPercentage = v; save(); Q_EMIT changed(); }
    bool clock24h() const { return m_clock24h; }
    void setClock24h(const bool &v) { if (m_clock24h == v) return; m_clock24h = v; save(); Q_EMIT changed(); }
    QString wallpaper() const { return m_wallpaper; }
    void setWallpaper(const QString &v) { if (m_wallpaper == v) return; m_wallpaper = v; save(); Q_EMIT changed(); }
    QString dynamicWallpaper() const { return m_dynamicWallpaper; }
    void setDynamicWallpaper(const QString &v) { if (m_dynamicWallpaper == v) return; m_dynamicWallpaper = v; save(); Q_EMIT changed(); }

private:
    QString m_weatherCity;
    double m_weatherLatitude;
    double m_weatherLongitude;
    QStringList m_desktopWidgets;
    int m_dockIconSize;
    bool m_dockMagnification;
    int m_dockMagnifiedSize;
    QString m_dockPosition;
    bool m_dockAutoHide;
    bool m_dockShowRecents;
    QStringList m_pinnedApps;
    bool m_menuBarBackground;
    bool m_showBatteryPercentage;
    bool m_clock24h;
    QString m_wallpaper;
    QString m_dynamicWallpaper;


public:
    static ShellSettings *instance();
    static ShellSettings *create(QQmlEngine *, QJSEngine *);
    Q_INVOKABLE void pin(const QString &launcherUrl);
    Q_INVOKABLE void unpin(const QString &launcherUrl);
    Q_INVOKABLE bool isPinned(const QString &launcherUrl) const;

Q_SIGNALS:
    void changed();

private:
    explicit ShellSettings(QObject *parent = nullptr);
    void load();
    void save();
    QFileSystemWatcher m_watcher;
    bool m_saving = false;
};
}
