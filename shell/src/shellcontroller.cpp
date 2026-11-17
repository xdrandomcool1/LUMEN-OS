// SPDX-License-Identifier: GPL-3.0-or-later
#include "shellcontroller.h"

#include <KGlobalAccel>
#include <QAction>
#include <QDBusPendingCall>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QKeySequence>
#include <QProcess>
#include <QQmlEngine>

namespace Lumen {

ShellController::ShellController(QObject *parent)
    : QObject(parent)
{
}

ShellController *ShellController::instance()
{
    static ShellController *s = new ShellController;
    return s;
}

ShellController *ShellController::create(QQmlEngine *, QJSEngine *)
{
    auto *c = instance();
    QQmlEngine::setObjectOwnership(c, QQmlEngine::CppOwnership);
    return c;
}

QString ShellController::Version() const
{
    return QStringLiteral(LUMEN_VERSION);
}

void ShellController::setActivePanel(const QString &p)
{
    if (m_activePanel == p)
        return;
    m_activePanel = p;
    Q_EMIT activePanelChanged();
}

void ShellController::toggle(const QString &panel)
{
    setActivePanel(m_activePanel == panel ? QString() : panel);
}

bool ShellController::registerDBus()
{
    auto bus = QDBusConnection::sessionBus();
    if (!bus.isConnected())
        return false;
    bus.registerObject(QStringLiteral("/Shell"), this, QDBusConnection::ExportScriptableSlots);
    return bus.registerService(QStringLiteral("org.lumen.Shell"));
}

void ShellController::registerShortcuts()
{
    struct Def { const char *id; const char *text; QKeySequence key; void (ShellController::*slot)(); };
    const Def defs[] = {
        {"toggle-spotlight", "Show Spotlight", QKeySequence(Qt::META | Qt::Key_Space), &ShellController::ToggleSpotlight},
        {"toggle-control-center", "Show Control Center", QKeySequence(Qt::META | Qt::Key_C), &ShellController::ToggleControlCenter},
        {"toggle-notification-center", "Show Notification Center", QKeySequence(Qt::META | Qt::Key_N), &ShellController::ToggleNotificationCenter},
        {"toggle-launchpad", "Show Launchpad", QKeySequence(Qt::META | Qt::Key_A), &ShellController::ToggleLaunchpad},
    };
    for (const auto &d : defs) {
        auto *a = new QAction(this);
        a->setObjectName(QLatin1String(d.id));
        a->setText(QLatin1String(d.text));
        a->setProperty("componentName", QStringLiteral("lumen-shell"));
        a->setProperty("componentDisplayName", QStringLiteral("LUMEN Shell"));
        KGlobalAccel::self()->setDefaultShortcut(a, {d.key});
        KGlobalAccel::self()->setShortcut(a, {d.key});
        connect(a, &QAction::triggered, this, d.slot);
    }
}

void ShellController::callLogind(const QString &method)
{
    QDBusMessage m = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.login1"),
                                                    QStringLiteral("/org/freedesktop/login1"),
                                                    QStringLiteral("org.freedesktop.login1.Manager"), method);
    m << true; // interactive: polkit may prompt for authorization
    QDBusConnection::systemBus().asyncCall(m);
}

bool ShellController::callKsmserver(const QString &method)
{
    // In a full session ksmserver asks running apps to save state first.
    auto *iface = QDBusConnection::sessionBus().interface();
    if (!iface || !iface->isServiceRegistered(QStringLiteral("org.kde.Shutdown")))
        return false;
    QDBusMessage m = QDBusMessage::createMethodCall(QStringLiteral("org.kde.Shutdown"), QStringLiteral("/Shutdown"),
                                                    QStringLiteral("org.kde.Shutdown"), method);
    QDBusConnection::sessionBus().asyncCall(m);
    return true;
}

void ShellController::lockScreen()
{
    QDBusMessage m = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.ScreenSaver"), QStringLiteral("/ScreenSaver"),
                                                    QStringLiteral("org.freedesktop.ScreenSaver"), QStringLiteral("Lock"));
    QDBusConnection::sessionBus().asyncCall(m);
}

void ShellController::suspend() { callLogind(QStringLiteral("Suspend")); }
void ShellController::logout() { if (!callKsmserver(QStringLiteral("logout"))) QProcess::startDetached(QStringLiteral("loginctl"), {QStringLiteral("terminate-session"), qEnvironmentVariable("XDG_SESSION_ID")}); }
void ShellController::reboot() { if (!callKsmserver(QStringLiteral("logoutAndReboot"))) callLogind(QStringLiteral("Reboot")); }
void ShellController::shutdown() { if (!callKsmserver(QStringLiteral("logoutAndShutdown"))) callLogind(QStringLiteral("PowerOff")); }

void ShellController::openSettings(const QString &page)
{
    QStringList args;
    if (!page.isEmpty())
        args << QStringLiteral("--page") << page;
    QProcess::startDetached(QStringLiteral("lumen-settings"), args);
    setActivePanel(QString());
}

static void invokeKWinShortcut(const QString &name)
{
    QDBusMessage m = QDBusMessage::createMethodCall(QStringLiteral("org.kde.kglobalaccel"), QStringLiteral("/component/kwin"),
                                                    QStringLiteral("org.kde.kglobalaccel.Component"), QStringLiteral("invokeShortcut"));
    m << name;
    QDBusConnection::sessionBus().asyncCall(m);
}

void ShellController::showDesktop() { invokeKWinShortcut(QStringLiteral("Show Desktop")); }
void ShellController::showOverview() { setActivePanel(QString()); invokeKWinShortcut(QStringLiteral("Lumen Mission Control")); }

void ShellController::takeScreenshot()
{
    setActivePanel(QString());
    QProcess::startDetached(QStringLiteral("lumen-screenshot"), {});
}

} // namespace Lumen
