// SPDX-License-Identifier: GPL-3.0-or-later
// Lumen Settings.
#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QQmlApplicationEngine>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("lumen-settings"));
    app.setApplicationDisplayName(QStringLiteral("System Settings"));
    app.setOrganizationName(QStringLiteral("lumen"));
    app.setDesktopFileName(QStringLiteral("lumen-settings"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("lumen-settings")));
    QCommandLineParser p;
    p.addHelpOption();
    QCommandLineOption page(QStringLiteral("page"), QStringLiteral("Pane to open"), QStringLiteral("id"), QStringLiteral("appearance"));
    p.addOption(page);
    p.process(app);
    QQmlApplicationEngine engine;
    engine.setInitialProperties({{QStringLiteral("startPage"), p.value(page).isEmpty() ? QStringLiteral("appearance") : p.value(page)}});
    engine.loadFromModule("Lumen.Settings", "Main");
    return engine.rootObjects().isEmpty() ? 1 : app.exec();
}
