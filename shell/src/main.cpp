// SPDX-License-Identifier: GPL-3.0-or-later
// lumen-shell — the LUMEN desktop shell (menu bar, Dock, panels, wallpaper).
#include "shellcontroller.h"
#include "themesync.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>

#include <notificationmanager/notifications.h>
#include <notificationmanager/server.h>
#include <notificationmanager/settings.h>
#include <taskmanager/abstracttasksmodel.h>
#include <taskmanager/tasksmodel.h>
#include <taskmanager/virtualdesktopinfo.h>

int main(int argc, char **argv)
{
    // Glass surfaces need per-pixel alpha on every shell window.
    QQuickWindow::setDefaultAlphaBuffer(true);
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("lumen-shell"));
    app.setOrganizationName(QStringLiteral("lumen"));
    app.setApplicationVersion(QStringLiteral(LUMEN_VERSION));
    // Must match lumen-shell.desktop: KWin grants the privileged window
    // management protocol (Dock window list) to that desktop file only.
    app.setDesktopFileName(QStringLiteral("lumen-shell"));
    app.setQuitOnLastWindowClosed(false);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption replace(QStringLiteral("replace"), QStringLiteral("Replace a running shell."));
    QCommandLineOption noNotifications(QStringLiteral("no-notification-server"), QStringLiteral("Do not register the notification server."));
    QCommandLineOption panel(QStringLiteral("panel"), QStringLiteral("Open a panel on start (testing)."), QStringLiteral("name"));
    parser.addOptions({replace, noNotifications, panel});
    parser.process(app);

    auto *controller = Lumen::ShellController::instance();
    auto *iface = QDBusConnection::sessionBus().interface();
    if (iface && iface->isServiceRegistered(QStringLiteral("org.lumen.Shell"))) {
        if (!parser.isSet(replace)) {
            qWarning("lumen-shell is already running (use --replace)");
            return 0;
        }
        iface->unregisterService(QStringLiteral("org.lumen.Shell"));
    }
    controller->registerDBus();
    controller->registerShortcuts();

    // Plasma workspace libraries used as building blocks (C++ only; their
    // QML wrappers belong to plasmashell, which LUMEN does not ship).
    const char *uri = "Lumen.Shell.Plasma";
    qmlRegisterType<TaskManager::TasksModel>(uri, 1, 0, "TasksModel");
    qmlRegisterUncreatableType<TaskManager::AbstractTasksModel>(uri, 1, 0, "AbstractTasksModel", QStringLiteral("enums only"));
    qmlRegisterType<TaskManager::VirtualDesktopInfo>(uri, 1, 0, "VirtualDesktopInfo");
    qmlRegisterType<NotificationManager::Notifications>(uri, 1, 0, "Notifications");
    qmlRegisterType<NotificationManager::Settings>(uri, 1, 0, "NotificationSettings");

    if (!parser.isSet(noNotifications)) {
        if (!NotificationManager::Server::self().init())
            qWarning("Could not register the notification server (another one is running?)");
    }

    // Keep third-party toolkits in sync with LUMEN's appearance.
    Lumen::ThemeSync themeSync;

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule("Lumen.Shell", "Shell");
    if (parser.isSet(panel))
        controller->setActivePanel(parser.value(panel));
    return app.exec();
}
