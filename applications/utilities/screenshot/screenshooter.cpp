// SPDX-License-Identifier: GPL-3.0-or-later
#include "screenshooter.h"
#include "capture.h"

#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>

namespace Lumen {
static QImage s_frozen;

Screenshooter::Screenshooter(QObject *parent) : QObject(parent) {}
QImage Screenshooter::frozen() { return s_frozen; }

void Screenshooter::save(const QImage &img)
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + QStringLiteral("/Screenshots");
    QDir().mkpath(dir);
    const QString path = dir + QStringLiteral("/Screenshot %1.png").arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd 'at' HH.mm.ss")));
    if (!img.save(path)) { m_error = tr("Could not save %1").arg(path); Q_EMIT failed(); return; }
    m_lastPath = path;
    Q_EMIT captured(path);
}

void Screenshooter::capture(const QString &mode, int delayMs)
{
    QTimer::singleShot(qMax(150, delayMs), this, [this, mode] {
        QString err;
        const auto m = mode == QLatin1String("window") ? Capture::Mode::ActiveWindow : Capture::Mode::Workspace;
        const QImage img = Capture::grab(m, {}, false, &err);
        if (img.isNull()) { m_error = err; Q_EMIT failed(); return; }
        if (mode == QLatin1String("freeze")) { s_frozen = img; Q_EMIT frozenReady(); return; }
        save(img);
    });
}

void Screenshooter::saveCrop(const QRectF &rect)
{
    if (s_frozen.isNull()) return;
    const qreal dpr = s_frozen.devicePixelRatio();
    const QRect px = QRectF(rect.x() * dpr, rect.y() * dpr, rect.width() * dpr, rect.height() * dpr).toAlignedRect();
    QImage c = s_frozen.copy(px);
    c.setDevicePixelRatio(dpr);
    save(c);
}

void Screenshooter::openLast()
{
    if (!m_lastPath.isEmpty()) QDesktopServices::openUrl(QUrl::fromLocalFile(m_lastPath));
}

QImage FrozenProvider::requestImage(const QString &, QSize *size, const QSize &)
{
    const QImage img = Screenshooter::frozen();
    if (size) *size = img.size();
    return img;
}
}
