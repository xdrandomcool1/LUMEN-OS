// SPDX-License-Identifier: GPL-3.0-or-later
// lumen-shell — global shell state, D-Bus interface and shortcuts.
#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QtQml/qqmlregistration.h>

namespace Lumen {

/*!
 * Single entry point for "open this panel" requests, whether they come from
 * a global shortcut (Super+Space …), D-Bus (org.lumen.Shell — used by
 * KWin gestures, apps and scripts) or the UI itself.
 */
class ShellController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_CLASSINFO("D-Bus Interface", "org.lumen.Shell")
    Q_PROPERTY(QString activePanel READ activePanel WRITE setActivePanel NOTIFY activePanelChanged)

public:
    static ShellController *instance();
    static ShellController *create(QQmlEngine *, QJSEngine *);

    QString activePanel() const { return m_activePanel; }
    void setActivePanel(const QString &p);

    void registerShortcuts();
    bool registerDBus();

public Q_SLOTS:
    // D-Bus API (also callable from QML)
    Q_SCRIPTABLE void ToggleSpotlight() { toggle(QStringLiteral("spotlight")); }
    Q_SCRIPTABLE void ToggleControlCenter() { toggle(QStringLiteral("controlcenter")); }
    Q_SCRIPTABLE void ToggleNotificationCenter() { toggle(QStringLiteral("notifications")); }
    Q_SCRIPTABLE void ToggleLaunchpad() { toggle(QStringLiteral("launchpad")); }
    Q_SCRIPTABLE void ShowPanel(const QString &name) { setActivePanel(name); }
    Q_SCRIPTABLE void ClosePanels() { setActivePanel(QString()); }
    Q_SCRIPTABLE QString Version() const;

    // Session / power actions (privileged parts go through logind/ksmserver).
    Q_INVOKABLE void lockScreen();
    Q_INVOKABLE void suspend();
    Q_INVOKABLE void logout();
    Q_INVOKABLE void reboot();
    Q_INVOKABLE void shutdown();
    Q_INVOKABLE void openSettings(const QString &page = QString());
    Q_INVOKABLE void showDesktop();
    Q_INVOKABLE void showOverview();
    Q_INVOKABLE void takeScreenshot();

Q_SIGNALS:
    void activePanelChanged();

private:
    explicit ShellController(QObject *parent = nullptr);
    void toggle(const QString &panel);
    void callLogind(const QString &method);
    bool callKsmserver(const QString &method);

    QString m_activePanel;
};

} // namespace Lumen
