// SPDX-License-Identifier: GPL-3.0-or-later
#include "battery.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusVariant>

namespace Lumen {
static const QString kSvc = QStringLiteral("org.freedesktop.UPower");
static const QString kDev = QStringLiteral("/org/freedesktop/UPower/devices/DisplayDevice");

Battery::Battery(QObject *parent) : QObject(parent)
{
    auto bus = QDBusConnection::systemBus();
    bus.connect(kSvc, kDev, QStringLiteral("org.freedesktop.DBus.Properties"), QStringLiteral("PropertiesChanged"), this, SLOT(refresh()));
    bus.connect(kSvc, QStringLiteral("/org/freedesktop/UPower"), QStringLiteral("org.freedesktop.DBus.Properties"), QStringLiteral("PropertiesChanged"), this, SLOT(refresh()));
    refresh();
}

void Battery::refresh()
{
    QDBusInterface props(kSvc, kDev, QStringLiteral("org.freedesktop.DBus.Properties"), QDBusConnection::systemBus());
    QDBusReply<QVariantMap> r = props.call(QStringLiteral("GetAll"), QStringLiteral("org.freedesktop.UPower.Device"));
    if (r.isValid()) {
        const auto m = r.value();
        // Type 2 = battery. IsPresent is false on desktops without batteries.
        m_present = m.value(QStringLiteral("IsPresent")).toBool() && m.value(QStringLiteral("Type")).toUInt() == 2;
        m_percentage = m.value(QStringLiteral("Percentage")).toDouble();
        m_state = m.value(QStringLiteral("State")).toUInt();
        m_tte = m.value(QStringLiteral("TimeToEmpty")).toLongLong();
        m_ttf = m.value(QStringLiteral("TimeToFull")).toLongLong();
    } else {
        m_present = false;
    }
    QDBusInterface up(kSvc, QStringLiteral("/org/freedesktop/UPower"), QStringLiteral("org.freedesktop.DBus.Properties"), QDBusConnection::systemBus());
    QDBusReply<QDBusVariant> ac = up.call(QStringLiteral("Get"), QStringLiteral("org.freedesktop.UPower"), QStringLiteral("OnBattery"));
    m_onAc = ac.isValid() ? !ac.value().variant().toBool() : true;
    Q_EMIT changed();
}

QString Battery::symbol() const
{
    if (charging()) return QStringLiteral("battery-charging");
    if (m_percentage > 85) return QStringLiteral("battery-full");
    if (m_percentage > 60) return QStringLiteral("battery-high");
    if (m_percentage > 35) return QStringLiteral("battery-medium");
    if (m_percentage > 12) return QStringLiteral("battery-low");
    return QStringLiteral("battery-warning");
}
}
