// SPDX-License-Identifier: GPL-3.0-or-later
// Lumen Finder — file manager.
#include <KFilePlacesModel>
#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("lumen-finder"));
    app.setApplicationDisplayName(QStringLiteral("Finder"));
    app.setOrganizationName(QStringLiteral("lumen"));
    app.setDesktopFileName(QStringLiteral("lumen-finder"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("lumen-finder")));

    QCommandLineParser p;
    p.addHelpOption();
    p.addPositionalArgument(QStringLiteral("url"), QStringLiteral("Folder to open"));
    p.process(app);
    QUrl start = QUrl::fromLocalFile(QDir::homePath());
    if (!p.positionalArguments().isEmpty()) {
        const QString a = p.positionalArguments().first();
        start = a.contains(QLatin1String(":/")) ? QUrl(a) : QUrl::fromUserInput(a, QDir::currentPath(), QUrl::AssumeLocalFile);
    }

    qmlRegisterType<KFilePlacesModel>("Lumen.Finder.Kio", 1, 0, "PlacesModel");
    QQmlApplicationEngine engine;
    engine.setInitialProperties({{QStringLiteral("startUrl"), start}});
    engine.loadFromModule("Lumen.Finder", "Main");
    return engine.rootObjects().isEmpty() ? 1 : app.exec();
}
