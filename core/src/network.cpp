// SPDX-License-Identifier: GPL-3.0-or-later
#include "network.h"

#include <NetworkManagerQt/AccessPoint>
#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessNetwork>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/WirelessSetting>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

namespace Lumen {

static NetworkManager::WirelessDevice::Ptr wifiDevice()
{
    for (const auto &d : NetworkManager::networkInterfaces())
        if (d->type() == NetworkManager::Device::Wifi)
            return d.objectCast<NetworkManager::WirelessDevice>();
    return {};
}

Network::Network(QObject *parent) : QObject(parent)
{
    m_debounce.setSingleShot(true);
    m_debounce.setInterval(150);
    connect(&m_debounce, &QTimer::timeout, this, [this] { refresh(); refreshNetworks(); });
    auto *n = NetworkManager::notifier();
    auto kick = [this] { m_debounce.start(); };
    connect(n, &NetworkManager::Notifier::wirelessEnabledChanged, this, kick);
    connect(n, &NetworkManager::Notifier::primaryConnectionChanged, this, kick);
    connect(n, &NetworkManager::Notifier::statusChanged, this, kick);
    connect(n, &NetworkManager::Notifier::activeConnectionsChanged, this, kick);
    connect(n, &NetworkManager::Notifier::deviceAdded, this, [this, kick] { watchDevices(); kick(); });
    connect(n, &NetworkManager::Notifier::deviceRemoved, this, kick);
    watchDevices();
    refresh();
    refreshNetworks();
}

void Network::watchDevices()
{
    if (auto w = wifiDevice()) {
        auto kick = [this] { m_debounce.start(); };
        connect(w.data(), &NetworkManager::WirelessDevice::networkAppeared, this, kick, Qt::UniqueConnection);
        connect(w.data(), &NetworkManager::WirelessDevice::networkDisappeared, this, kick, Qt::UniqueConnection);
        connect(w.data(), &NetworkManager::WirelessDevice::activeAccessPointChanged, this, kick, Qt::UniqueConnection);
    }
}

bool Network::available() const { return NetworkManager::status() != NetworkManager::Unknown; }
bool Network::wifiAvailable() const { return !wifiDevice().isNull(); }
bool Network::wifiEnabled() const { return NetworkManager::isWirelessEnabled(); }
void Network::setWifiEnabled(bool e) { NetworkManager::setWirelessEnabled(e); }

void Network::setError(const QString &e)
{
    if (m_lastError == e) return;
    m_lastError = e;
    Q_EMIT lastErrorChanged();
}

void Network::refresh()
{
    m_kind = QStringLiteral("none");
    m_connectionName.clear();
    m_signal = 0;
    m_ip.clear();
    const auto primary = NetworkManager::primaryConnection();
    if (primary) {
        m_connectionName = primary->id();
        const auto type = primary->type();
        if (type == NetworkManager::ConnectionSettings::Wireless) {
            m_kind = QStringLiteral("wifi");
            if (auto w = wifiDevice(); w && w->activeAccessPoint())
                m_signal = w->activeAccessPoint()->signalStrength();
        } else if (type == NetworkManager::ConnectionSettings::Wired) {
            m_kind = QStringLiteral("ethernet");
        } else {
            m_kind = QStringLiteral("other");
        }
        const auto devs = primary->devices();
        if (!devs.isEmpty()) {
            const auto dev = NetworkManager::findNetworkInterface(devs.first());
            if (dev && !dev->ipV4Config().addresses().isEmpty())
                m_ip = dev->ipV4Config().addresses().first().ip().toString();
        }
    }
    Q_EMIT changed();
}

bool Network::isKnown(const QString &ssid) const
{
    for (const auto &c : NetworkManager::listConnections()) {
        const auto s = c->settings();
        if (s->connectionType() != NetworkManager::ConnectionSettings::Wireless) continue;
        const auto w = s->setting(NetworkManager::Setting::Wireless).staticCast<NetworkManager::WirelessSetting>();
        if (w && QString::fromUtf8(w->ssid()) == ssid) return true;
    }
    return false;
}

void Network::refreshNetworks()
{
    m_networks.clear();
    const auto w = wifiDevice();
    if (w && wifiEnabled()) {
        const auto active = w->activeAccessPoint();
        QList<QVariantMap> list;
        for (const auto &net : w->networks()) {
            const auto ap = net->referenceAccessPoint();
            if (!ap || net->ssid().isEmpty()) continue;
            const bool secure = ap->wpaFlags() || ap->rsnFlags() || (ap->capabilities() & NetworkManager::AccessPoint::Privacy);
            list.append({{QStringLiteral("ssid"), net->ssid()},
                         {QStringLiteral("strength"), net->signalStrength()},
                         {QStringLiteral("secure"), secure},
                         {QStringLiteral("active"), active && active->ssid() == net->ssid()},
                         {QStringLiteral("known"), isKnown(net->ssid())}});
        }
        std::sort(list.begin(), list.end(), [](const QVariantMap &a, const QVariantMap &b) {
            if (a[QStringLiteral("active")] != b[QStringLiteral("active")]) return a[QStringLiteral("active")].toBool();
            if (a[QStringLiteral("known")] != b[QStringLiteral("known")]) return a[QStringLiteral("known")].toBool();
            return a[QStringLiteral("strength")].toInt() > b[QStringLiteral("strength")].toInt();
        });
        for (const auto &m : list) m_networks.append(m);
    }
    Q_EMIT networksChanged();
}

void Network::scan()
{
    if (auto w = wifiDevice())
        w->requestScan();
}

void Network::connectTo(const QString &ssid, const QString &password)
{
    const auto w = wifiDevice();
    if (!w) { setError(tr("No Wi-Fi device")); return; }
    setError(QString());
    for (const auto &c : NetworkManager::listConnections()) {
        const auto s = c->settings();
        if (s->connectionType() != NetworkManager::ConnectionSettings::Wireless) continue;
        const auto ws = s->setting(NetworkManager::Setting::Wireless).staticCast<NetworkManager::WirelessSetting>();
        if (ws && QString::fromUtf8(ws->ssid()) == ssid) {
            NetworkManager::activateConnection(c->path(), w->uni(), QString());
            return;
        }
    }
    const auto net = w->findNetwork(ssid);
    const auto ap = net ? net->referenceAccessPoint() : NetworkManager::AccessPoint::Ptr();
    NetworkManager::ConnectionSettings::Ptr settings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    settings->setId(ssid);
    settings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());
    settings->setAutoconnect(true);
    auto ws = settings->setting(NetworkManager::Setting::Wireless).staticCast<NetworkManager::WirelessSetting>();
    ws->setSsid(ssid.toUtf8());
    ws->setMode(NetworkManager::WirelessSetting::Infrastructure);
    ws->setInitialized(true);
    if (!password.isEmpty()) {
        auto sec = settings->setting(NetworkManager::Setting::WirelessSecurity).staticCast<NetworkManager::WirelessSecuritySetting>();
        sec->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaPsk);
        sec->setPsk(password);
        sec->setInitialized(true);
    }
    auto reply = NetworkManager::addAndActivateConnection(settings->toMap(), w->uni(), ap ? ap->uni() : QString());
    auto *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *wch) {
        if (wch->isError()) setError(wch->error().message());
        wch->deleteLater();
    });
}

void Network::disconnectWifi()
{
    if (auto w = wifiDevice())
        w->disconnectInterface();
}

QString Network::symbol() const
{
    if (m_kind == QLatin1String("ethernet")) return QStringLiteral("plugs-connected");
    if (m_kind == QLatin1String("wifi")) {
        if (m_signal > 70) return QStringLiteral("wifi-high");
        if (m_signal > 40) return QStringLiteral("wifi-medium");
        return QStringLiteral("wifi-low");
    }
    if (!wifiEnabled()) return QStringLiteral("wifi-slash");
    return QStringLiteral("wifi-none");
}
}
