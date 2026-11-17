// SPDX-License-Identifier: GPL-3.0-or-later
// Lumen Terminal.
#include <QCommandLineParser>
#include <QDir>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("lumen-terminal"));
    app.setApplicationDisplayName(QStringLiteral("Terminal"));
    app.setOrganizationName(QStringLiteral("lumen"));
    app.setDesktopFileName(QStringLiteral("lumen-terminal"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("lumen-terminal")));
    QCommandLineParser p;
    p.addHelpOption();
    QCommandLineOption wd({QStringLiteral("w"), QStringLiteral("workdir")}, QStringLiteral("Working directory"), QStringLiteral("dir"));
    p.addOption(wd);
    QCommandLineOption exec(QStringLiteral("e"), QStringLiteral("Command to run instead of the shell (rest of the arguments)."));
    p.addOption(exec);
    p.addPositionalArgument(QStringLiteral("command"), QStringLiteral("With -e: command and arguments"));
    // Everything after the command name belongs to the command (-e bash -c …).
    p.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsPositionalArguments);
    p.process(app);
    QQmlApplicationEngine engine;
    engine.setInitialProperties({{QStringLiteral("startDir"), p.isSet(wd) ? p.value(wd) : QDir::homePath()},
                                 {QStringLiteral("startCommand"), p.isSet(exec) ? p.positionalArguments() : QStringList()}});
    engine.loadFromModule("Lumen.Terminal", "Main");
    return engine.rootObjects().isEmpty() ? 1 : app.exec();
}
