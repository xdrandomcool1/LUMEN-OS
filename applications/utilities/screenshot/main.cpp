// SPDX-License-Identifier: GPL-3.0-or-later
// lumen-screenshot — capture the screen, a window or an area.
//
//   lumen-screenshot                    interactive (GUI)
//   lumen-screenshot -o shot.png        full workspace, no UI
//   lumen-screenshot -w -o win.png      active window
//   lumen-screenshot -a 0,0,800,600 -o area.png
#include "capture.h"
#include "screenshooter.h"

#include <QCommandLineParser>
#include <QDateTime>
#include <QDir>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QStandardPaths>
#include <QThread>
#include <cstdio>

using namespace Lumen;

static QString defaultPath()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + QStringLiteral("/Screenshots");
    QDir().mkpath(dir);
    return dir + QStringLiteral("/Screenshot %1.png").arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd 'at' HH.mm.ss")));
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("lumen-screenshot"));
    app.setDesktopFileName(QStringLiteral("lumen-screenshot"));
    QCommandLineParser p;
    p.addHelpOption();
    QCommandLineOption out({QStringLiteral("o"), QStringLiteral("output")}, QStringLiteral("Write to file and exit."), QStringLiteral("file"));
    QCommandLineOption win({QStringLiteral("w"), QStringLiteral("window")}, QStringLiteral("Capture the active window."));
    QCommandLineOption area({QStringLiteral("a"), QStringLiteral("area")}, QStringLiteral("Capture x,y,w,h."), QStringLiteral("rect"));
    QCommandLineOption delay({QStringLiteral("d"), QStringLiteral("delay")}, QStringLiteral("Delay in ms."), QStringLiteral("ms"), QStringLiteral("0"));
    QCommandLineOption cursor(QStringLiteral("cursor"), QStringLiteral("Include the pointer."));
    p.addOptions({out, win, area, delay, cursor});
    p.process(app);

    const bool cli = p.isSet(out);
    if (cli) {
        QThread::msleep(p.value(delay).toUInt());
        Capture::Mode mode = Capture::Mode::Workspace;
        QRect r;
        if (p.isSet(win)) mode = Capture::Mode::ActiveWindow;
        if (p.isSet(area)) {
            const auto v = p.value(area).split(QLatin1Char(','));
            r = QRect(v.value(0).toInt(), v.value(1).toInt(), v.value(2).toInt(), v.value(3).toInt());
            mode = Capture::Mode::Area;
        }
        QString err;
        const QImage img = Capture::grab(mode, r, p.isSet(cursor), &err);
        if (img.isNull()) {
            std::fprintf(stderr, "capture failed: %s\n", qPrintable(err));
            return 1;
        }
        const QString file = p.value(out).isEmpty() ? defaultPath() : p.value(out);
        if (!img.save(file)) {
            std::fprintf(stderr, "cannot write %s\n", qPrintable(file));
            return 1;
        }
        std::printf("%s (%dx%d)\n", qPrintable(file), img.width(), img.height());
        return 0;
    }

    QQmlApplicationEngine engine;
    engine.addImageProvider(QStringLiteral("frozen"), new Lumen::FrozenProvider);
    engine.loadFromModule("Lumen.Screenshot", "Main");
    return engine.rootObjects().isEmpty() ? 1 : app.exec();
}
