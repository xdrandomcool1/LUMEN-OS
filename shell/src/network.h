// SPDX-License-Identifier: GPL-3.0-or-later
// NetworkManager integration: status, Wi-Fi list, connect/disconnect.
#pragma once
#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

namespace Lumen {
class Network : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool available READ available NOTIFY changed)
    Q_PROPERTY(bool wifiAvailable READ wifiAvailable NOTIFY changed)
    Q_PROPERTY(bool wifiEnabled READ wifiEnabled WRITE setWifiEnabled NOTIFY changed)
    Q_PROPERTY(QString kind READ kind NOTIFY changed)            // wifi | ethernet | other | none
    Q_PROPERTY(QString connectionName READ connectionName NOTIFY changed)
    Q_PROPERTY(int signal READ signal NOTIFY changed)
    Q_PROPERTY(QString ipAddress READ ipAddress NOTIFY changed)
    Q_PROPERTY(QString symbol READ symbol NOTIFY changed)
    Q_PROPERTY(QVariantList networks READ networks NOTIFY networksChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
public:
    explicit Network(QObject *parent = nullptr);
    bool available() const;
    bool wifiAvailable() const;
    bool wifiEnabled() const;
    void setWifiEnabled(bool e);
    QString kind() const { return m_kind; }
    QString connectionName() const { return m_connectionName; }
    int signal() const { return m_signal; }
    QString ipAddress() const { return m_ip; }
    QString symbol() const;
    QVariantList networks() const { return m_networks; }
    QString lastError() const { return m_lastError; }

    Q_INVOKABLE void scan();
    //! Connects to \a ssid, reusing a saved profile when one exists.
    //! For new secured networks \a password is the WPA passphrase.
    Q_INVOKABLE void connectTo(const QString &ssid, const QString &password = QString());
    Q_INVOKABLE void disconnectWifi();
    Q_INVOKABLE bool isKnown(const QString &ssid) const;
Q_SIGNALS:
    void changed();
    void networksChanged();
    void lastErrorChanged();
private:
    void refresh();
    void refreshNetworks();
    void watchDevices();
    void setError(const QString &e);
    QString m_kind = QStringLiteral("none");
    QString m_connectionName;
    int m_signal = 0;
    QString m_ip;
    QVariantList m_networks;
    QString m_lastError;
    QTimer m_debounce;
};
}
