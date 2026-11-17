// SPDX-License-Identifier: GPL-3.0-or-later
// Lumen Settings — system back-ends. Every privileged change goes through a
// polkit-guarded D-Bus service (hostnamed, AccountsService, systemd); the
// Settings app itself never runs privileged code.
#pragma once

#include <QObject>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>
#include <kscreen/types.h>

namespace Lumen {

/*! Read/write KDE-style config files with change notification, so KWin,
 *  PowerDevil and kscreenlocker pick changes up immediately. */
class KdeConfig : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
public:
    using QObject::QObject;
    Q_INVOKABLE QVariant read(const QString &file, const QString &group, const QString &key, const QVariant &def = {}) const;
    Q_INVOKABLE void write(const QString &file, const QString &group, const QString &key, const QVariant &value);
    Q_INVOKABLE void reconfigureKWin();
    Q_INVOKABLE void reloadKeyboard();
    Q_INVOKABLE void reloadScreenLocker();
    Q_INVOKABLE void reloadPowerDevil();
};

/*! Displays via libkscreen (resolution, scale, refresh rate, enable). */
class Displays : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QVariantList outputs READ outputs NOTIFY changed)
    Q_PROPERTY(bool ready READ ready NOTIFY changed)
public:
    explicit Displays(QObject *parent = nullptr);
    QVariantList outputs() const;
    bool ready() const;
    Q_INVOKABLE void setMode(int outputId, const QString &modeId);
    Q_INVOKABLE void setScale(int outputId, qreal scale);
    Q_INVOKABLE void setEnabled(int outputId, bool enabled);
    Q_INVOKABLE void reload();
Q_SIGNALS:
    void changed();
    void applyFailed(const QString &reason);
private:
    void apply();
    KScreen::ConfigPtr m_config;
};

/*! Pointer / touchpad settings exposed by KWin (persisted by KWin). */
class InputDevices : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QVariantList devices READ devices NOTIFY changed)
public:
    explicit InputDevices(QObject *parent = nullptr);
    QVariantList devices() const { return m_devices; }
    Q_INVOKABLE void set(const QString &sysName, const QString &property, const QVariant &value);
    Q_INVOKABLE void reload();
Q_SIGNALS:
    void changed();
private:
    QVariantList m_devices;
};

/*! Hardware/OS facts for "About" + hostname changes via hostnamed. */
class SystemInfo : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString hostname READ hostname NOTIFY changed)
    Q_PROPERTY(QVariantMap facts READ facts CONSTANT)
public:
    explicit SystemInfo(QObject *parent = nullptr);
    QString hostname() const;
    QVariantMap facts() const { return m_facts; }
    Q_INVOKABLE void setHostname(const QString &name);
    static QString gpuName();
    static QString lookupPciName(const QString &vendor, const QString &device);
Q_SIGNALS:
    void changed();
    void failed(const QString &message);
private:
    QVariantMap m_facts;
};

/*! The current user's account via AccountsService. */
class Accounts : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString userName READ userName NOTIFY changed)
    Q_PROPERTY(QString realName READ realName NOTIFY changed)
    Q_PROPERTY(QString iconFile READ iconFile NOTIFY changed)
    Q_PROPERTY(bool administrator READ administrator NOTIFY changed)
    Q_PROPERTY(QVariantList users READ users NOTIFY changed)
public:
    explicit Accounts(QObject *parent = nullptr);
    QString userName() const { return m_user.value(QStringLiteral("UserName")).toString(); }
    QString realName() const { return m_user.value(QStringLiteral("RealName")).toString(); }
    QString iconFile() const { return m_user.value(QStringLiteral("IconFile")).toString(); }
    bool administrator() const { return m_user.value(QStringLiteral("AccountType")).toInt() == 1; }
    QVariantList users() const { return m_users; }
    Q_INVOKABLE void setRealName(const QString &name);
    Q_INVOKABLE void setIconFile(const QString &path);
    //! Hashes with SHA-512 crypt and hands it to AccountsService (polkit asks).
    Q_INVOKABLE void setPassword(const QString &password, const QString &hint);
    Q_INVOKABLE void reload();
    static QString hashPassword(const QString &password);
Q_SIGNALS:
    void changed();
    void failed(const QString &message);
private:
    void call(const QString &method, const QVariantList &args);
    QString m_path;
    QVariantMap m_user;
    QVariantList m_users;
};

/*! Firewall (firewalld) state; start/stop via systemd + polkit. */
class Firewall : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool available READ available NOTIFY changed)
    Q_PROPERTY(bool running READ running NOTIFY changed)
    Q_PROPERTY(QString zone READ zone NOTIFY changed)
public:
    explicit Firewall(QObject *parent = nullptr);
    bool available() const { return m_available; }
    bool running() const { return m_running; }
    QString zone() const { return m_zone; }
    Q_INVOKABLE void setRunning(bool on);
    Q_INVOKABLE void refresh();
Q_SIGNALS:
    void changed();
    void failed(const QString &message);
private:
    bool m_available = false, m_running = false;
    QString m_zone;
};

/*! power-profiles-daemon (Low Power / Balanced / Performance). */
class PowerProfiles : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool available READ available NOTIFY changed)
    Q_PROPERTY(QString active READ active WRITE setActive NOTIFY changed)
    Q_PROPERTY(QStringList profiles READ profiles NOTIFY changed)
public:
    explicit PowerProfiles(QObject *parent = nullptr);
    bool available() const { return !m_profiles.isEmpty(); }
    QString active() const { return m_active; }
    void setActive(const QString &p);
    QStringList profiles() const { return m_profiles; }
    Q_INVOKABLE void refresh();
Q_SIGNALS:
    void changed();
private:
    QString m_service, m_path, m_iface;
    QString m_active;
    QStringList m_profiles;
};

/*! XKB layouts available on the system (evdev.lst) for the keyboard page. */
class XkbLayouts : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QVariantList all READ all CONSTANT)
public:
    explicit XkbLayouts(QObject *parent = nullptr);
    QVariantList all() const { return m_all; }
    static QVariantList parse(const QString &evdevLst);
private:
    QVariantList m_all;
};

} // namespace Lumen
