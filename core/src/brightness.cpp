// SPDX-License-Identifier: GPL-3.0-or-later
#include "brightness.h"
#include <QDBusPendingCall>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusVariant>

namespace Lumen {
static const QString kSvc = QStringLiteral("org.kde.ScreenBrightness");
static const QString kPath = QStringLiteral("/org/kde/ScreenBrightness");

Brightness::Brightness(QObject *parent) : QObject(parent)
{
    auto bus = QDBusConnection::sessionBus();
    bus.connect(kSvc, kPath, kSvc, QStringLiteral("BrightnessChanged"), this, SLOT(onBrightnessChanged(QString,int)));
    bus.connect(kSvc, kPath, kSvc, QStringLiteral("DisplayAdded"), this, SLOT(refresh()));
    bus.connect(kSvc, kPath, kSvc, QStringLiteral("DisplayRemoved"), this, SLOT(refresh()));
    refresh();
}

static QVariant prop(const QString &path, const QString &iface, const QString &name)
{
    QDBusInterface p(kSvc, path, QStringLiteral("org.freedesktop.DBus.Properties"), QDBusConnection::sessionBus());
    QDBusReply<QDBusVariant> r = p.call(QStringLiteral("Get"), iface, name);
    return r.isValid() ? r.value().variant() : QVariant();
}

void Brightness::refresh()
{
    m_display.clear();
    m_max = 0;
    const QStringList names = prop(kPath, kSvc, QStringLiteral("DisplaysDBusNames")).toStringList();
    // Prefer the internal panel; otherwise the first controllable display.
    for (const QString &n : names) {
        const QString path = kPath + QLatin1Char('/') + n;
        const QString di = kSvc + QStringLiteral(".Display");
        const bool internal = prop(path, di, QStringLiteral("IsInternal")).toBool();
        if (m_display.isEmpty() || internal) {
            m_display = n;
            m_label = prop(path, di, QStringLiteral("Label")).toString();
            m_max = prop(path, di, QStringLiteral("MaxBrightness")).toInt();
            m_value = prop(path, di, QStringLiteral("Brightness")).toInt();
            if (internal)
                break;
        }
    }
    Q_EMIT changed();
}

void Brightness::onBrightnessChanged(const QString &display, int value)
{
    if (display != m_display)
        return;
    m_value = value;
    Q_EMIT changed();
}

void Brightness::setValue(qreal v)
{
    if (!available())
        return;
    const int target = qBound(1, qRound(v * m_max), m_max); // never fully off
    if (target == m_value)
        return;
    m_value = target;
    QDBusInterface d(kSvc, kPath + QLatin1Char('/') + m_display, kSvc + QStringLiteral(".Display"), QDBusConnection::sessionBus());
    d.asyncCall(QStringLiteral("SetBrightness"), target, 0x1u /* SuppressIndicator: we draw our own OSD */);
    Q_EMIT changed();
}
}
