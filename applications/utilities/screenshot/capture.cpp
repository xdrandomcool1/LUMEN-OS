// SPDX-License-Identifier: GPL-3.0-or-later
#include "capture.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMetaType>
#include <QDBusMessage>
#include <QDBusUnixFileDescriptor>
#include <QGuiApplication>
#include <QScreen>
#include <cerrno>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

namespace Lumen::Capture {

static QByteArray readAll(int fd)
{
    QByteArray data;
    char buf[1 << 16];
    for (;;) {
        pollfd p{fd, POLLIN, 0};
        if (::poll(&p, 1, 5000) <= 0)
            break;
        const ssize_t n = ::read(fd, buf, sizeof buf);
        if (n > 0) data.append(buf, n);
        else if (n == 0) break;
        else if (errno != EINTR && errno != EAGAIN) break;
    }
    return data;
}

QImage grab(Mode mode, const QRect &area, bool includeCursor, QString *error)
{
    int fds[2];
    if (::pipe2(fds, O_CLOEXEC) != 0) {
        if (error) *error = QStringLiteral("pipe() failed");
        return {};
    }
    QVariantMap options{{QStringLiteral("include-cursor"), includeCursor},
                        {QStringLiteral("native-resolution"), true},
                        {QStringLiteral("include-decoration"), true},
                        {QStringLiteral("include-shadow"), false}};
    QString method;
    QVariantList args;
    switch (mode) {
    case Mode::Workspace: method = QStringLiteral("CaptureWorkspace"); args << options; break;
    case Mode::ActiveWindow: method = QStringLiteral("CaptureActiveWindow"); args << options; break;
    case Mode::Screen: {
        method = QStringLiteral("CaptureScreen");
        args << (QGuiApplication::primaryScreen() ? QGuiApplication::primaryScreen()->name() : QString()) << options;
        break;
    }
    case Mode::Area:
        method = QStringLiteral("CaptureArea");
        args << area.x() << area.y() << uint(area.width()) << uint(area.height()) << options;
        break;
    }
    args << QVariant::fromValue(QDBusUnixFileDescriptor(fds[1]));
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"), QStringLiteral("/org/kde/KWin/ScreenShot2"),
                                                      QStringLiteral("org.kde.KWin.ScreenShot2"), method);
    msg.setArguments(args);
    const QDBusMessage reply = QDBusConnection::sessionBus().call(msg, QDBus::Block, 10000);
    ::close(fds[1]);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        ::close(fds[0]);
        if (error) *error = reply.errorMessage();
        return {};
    }
    const QVariantMap res = qdbus_cast<QVariantMap>(reply.arguments().value(0));
    const QByteArray data = readAll(fds[0]);
    ::close(fds[0]);
    const int w = res.value(QStringLiteral("width")).toInt();
    const int h = res.value(QStringLiteral("height")).toInt();
    const int stride = res.value(QStringLiteral("stride")).toInt();
    const auto format = QImage::Format(res.value(QStringLiteral("format")).toInt());
    if (w <= 0 || h <= 0 || data.size() < qsizetype(stride) * h) {
        if (error) *error = QStringLiteral("incomplete image data (%1 bytes)").arg(data.size());
        return {};
    }
    QImage img(reinterpret_cast<const uchar *>(data.constData()), w, h, stride, format);
    QImage out = img.copy();
    const qreal scale = res.value(QStringLiteral("scale"), 1.0).toReal();
    out.setDevicePixelRatio(scale > 0 ? scale : 1.0);
    return out;
}
}
