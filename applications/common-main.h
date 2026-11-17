// SPDX-License-Identifier: GPL-3.0-or-later
// Shared entry point for LUMEN QML applications.
#pragma once
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>

namespace Lumen {
inline int runApp(int argc, char **argv, const char *id, const char *displayName, const char *uri,
                  QVariantMap (*initial)(const QStringList &) = nullptr)
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QString::fromLatin1(id));
    app.setApplicationDisplayName(QString::fromUtf8(displayName));
    app.setOrganizationName(QStringLiteral("lumen"));
    app.setDesktopFileName(QString::fromLatin1(id));
    app.setWindowIcon(QIcon::fromTheme(QString::fromLatin1(id)));
    QCommandLineParser p;
    p.addHelpOption();
    p.addPositionalArgument(QStringLiteral("files"), QStringLiteral("Files to open"));
    p.process(app);
    QQmlApplicationEngine engine;
    if (initial)
        engine.setInitialProperties(initial(p.positionalArguments()));
    engine.loadFromModule(uri, "Main");
    return engine.rootObjects().isEmpty() ? 1 : app.exec();
}
}
