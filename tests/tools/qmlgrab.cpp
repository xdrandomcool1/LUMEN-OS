// SPDX-License-Identifier: GPL-3.0-or-later
// lumen-qmlgrab — render a QML file in a real window and save screenshots.
//
//   lumen-qmlgrab scene.qml out.png [--size 1440x900] [--at 300,800,1500]
//                 [--dpr 2] [--eval "js"]
//
// --at takes timestamps (ms after the first frame); one PNG per timestamp is
// written as out-<ms>.png (or exactly out.png for a single timestamp).
// The root object may expose `function step(ms)` which is called before each
// capture, letting tests drive interactions deterministically.
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#include <QQuickWindow>
#include <QTimer>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QDir>
#include <cstdio>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QCommandLineParser p;
    p.addPositionalArgument("qml", "Scene");
    p.addPositionalArgument("out", "Output PNG");
    p.addOption({"size", "Window size", "WxH", "1440x900"});
    p.addOption({"at", "Capture times (ms)", "list", "1200"});
    p.addOption({"import", "Extra QML import path", "dir"});
    p.addOption({"prop", "Root property name=value (repeatable)", "kv"});
    p.addHelpOption();
    p.process(app);
    if (p.positionalArguments().size() < 2)
        p.showHelp(1);

    const QStringList sz = p.value("size").split('x');
    QQuickView view;
    view.setColor(Qt::black);
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    for (const QString &imp : p.values("import"))
        view.engine()->addImportPath(imp);
    view.resize(sz.value(0).toInt(), sz.value(1).toInt());
    view.setSource(QUrl::fromLocalFile(QFileInfo(p.positionalArguments().at(0)).absoluteFilePath()));
    if (view.status() != QQuickView::Ready) {
        for (const auto &e : view.errors())
            qCritical().noquote() << e.toString();
        return 2;
    }
    for (const QString &kv : p.values("prop")) {
        const int eq = kv.indexOf('=');
        view.rootObject()->setProperty(kv.left(eq).toUtf8(), kv.mid(eq + 1));
    }
    view.show();

    QList<int> times;
    for (const QString &t : p.value("at").split(','))
        times << t.toInt();
    std::sort(times.begin(), times.end());
    const QString out = p.positionalArguments().at(1);
    const QString base = out.endsWith(".png") ? out.chopped(4) : out;

    auto *clock = new QElapsedTimer;
    bool started = false;
    int idx = 0;
    QObject::connect(&view, &QQuickWindow::frameSwapped, &app, [&] {
        if (!started) { started = true; clock->start(); }
    });
    auto *tick = new QTimer(&app);
    tick->setInterval(8);
    QObject::connect(tick, &QTimer::timeout, &app, [&] {
        view.update(); // keep frames flowing so animations progress
        if (!started || idx >= times.size())
            return;
        if (clock->elapsed() < times[idx])
            return;
        QMetaObject::invokeMethod(view.rootObject(), "step", Q_ARG(QVariant, times[idx]));
        const QImage img = view.grabWindow();
        const QString file = times.size() == 1 ? base + ".png" : QStringLiteral("%1-%2.png").arg(base).arg(times[idx]);
        QDir().mkpath(QFileInfo(file).absolutePath());
        if (!img.save(file))
            std::fprintf(stderr, "failed to write %s\n", qPrintable(file));
        else
            std::fprintf(stdout, "wrote %s (%dx%d)\n", qPrintable(file), img.width(), img.height());
        if (++idx >= times.size())
            QTimer::singleShot(0, &app, &QCoreApplication::quit);
    });
    tick->start();
    QTimer::singleShot(60000, &app, [] { qCritical("timeout"); QCoreApplication::exit(3); });
    return app.exec();
}
