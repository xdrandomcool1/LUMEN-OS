// SPDX-License-Identifier: GPL-3.0-or-later
#include "backends.h"

#include <KConfig>
#include <KConfigGroup>
#include <KScreen/Config>
#include <KScreen/GetConfigOperation>
#include <KScreen/Mode>
#include <KScreen/Output>
#include <KScreen/SetConfigOperation>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QDBusVariant>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QStorageInfo>
#include <QSysInfo>
#include <QTextStream>
#include <QUrl>
#include <crypt.h>
#include <unistd.h>

namespace Lumen {

// ---------------------------------------------------------------- helpers
static QVariant dbusProp(const QDBusConnection &bus, const QString &svc, const QString &path, const QString &iface, const QString &name)
{
    QDBusMessage m = QDBusMessage::createMethodCall(svc, path, QStringLiteral("org.freedesktop.DBus.Properties"), QStringLiteral("Get"));
    m << iface << name;
    const QDBusMessage r = bus.call(m, QDBus::Block, 2000);
    if (r.type() != QDBusMessage::ReplyMessage || r.arguments().isEmpty())
        return {};
    return r.arguments().first().value<QDBusVariant>().variant();
}

static QVariantMap dbusAll(const QDBusConnection &bus, const QString &svc, const QString &path, const QString &iface)
{
    QDBusMessage m = QDBusMessage::createMethodCall(svc, path, QStringLiteral("org.freedesktop.DBus.Properties"), QStringLiteral("GetAll"));
    m << iface;
    const QDBusMessage r = bus.call(m, QDBus::Block, 2000);
    if (r.type() != QDBusMessage::ReplyMessage || r.arguments().isEmpty())
        return {};
    return qdbus_cast<QVariantMap>(r.arguments().first());
}

// ---------------------------------------------------------------- KdeConfig
QVariant KdeConfig::read(const QString &file, const QString &group, const QString &key, const QVariant &def) const
{
    KConfig cfg(file);
    KConfigGroup g = cfg.group(group);
    for (const QString &sub : group.split(QLatin1Char('/')).mid(1))
        Q_UNUSED(sub)
    if (group.contains(QLatin1Char('/'))) {
        const QStringList parts = group.split(QLatin1Char('/'));
        g = cfg.group(parts.first());
        for (int i = 1; i < parts.size(); ++i) g = g.group(parts[i]);
    }
    if (!g.hasKey(key))
        return def;
    switch (def.typeId()) {
    case QMetaType::Bool: return g.readEntry(key, def.toBool());
    case QMetaType::Int: return g.readEntry(key, def.toInt());
    case QMetaType::Double: return g.readEntry(key, def.toDouble());
    default: return g.readEntry(key, def.toString());
    }
}

void KdeConfig::write(const QString &file, const QString &group, const QString &key, const QVariant &value)
{
    KConfig cfg(file);
    const QStringList parts = group.split(QLatin1Char('/'));
    KConfigGroup g = cfg.group(parts.first());
    for (int i = 1; i < parts.size(); ++i) g = g.group(parts[i]);
    g.writeEntry(key, value, KConfig::Notify);
    cfg.sync();
}

void KdeConfig::reconfigureKWin()
{
    QDBusMessage m = QDBusMessage::createSignal(QStringLiteral("/KWin"), QStringLiteral("org.kde.KWin"), QStringLiteral("reloadConfig"));
    QDBusConnection::sessionBus().send(m);
}

void KdeConfig::reloadKeyboard()
{
    QDBusMessage m = QDBusMessage::createSignal(QStringLiteral("/Layouts"), QStringLiteral("org.kde.keyboard"), QStringLiteral("reloadConfig"));
    QDBusConnection::sessionBus().send(m);
}

void KdeConfig::reloadScreenLocker()
{
    QDBusMessage m = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.ScreenSaver"), QStringLiteral("/ScreenSaver"),
                                                    QStringLiteral("org.kde.screensaver"), QStringLiteral("configure"));
    QDBusConnection::sessionBus().asyncCall(m);
}

void KdeConfig::reloadPowerDevil()
{
    QDBusMessage m = QDBusMessage::createMethodCall(QStringLiteral("org.kde.Solid.PowerManagement"), QStringLiteral("/org/kde/Solid/PowerManagement"),
                                                    QStringLiteral("org.kde.Solid.PowerManagement"), QStringLiteral("refreshStatus"));
    QDBusConnection::sessionBus().asyncCall(m);
}

// ---------------------------------------------------------------- Displays
Displays::Displays(QObject *parent) : QObject(parent) { reload(); }

void Displays::reload()
{
    auto *op = new KScreen::GetConfigOperation();
    connect(op, &KScreen::ConfigOperation::finished, this, [this](KScreen::ConfigOperation *o) {
        if (!o->hasError())
            m_config = qobject_cast<KScreen::GetConfigOperation *>(o)->config();
        Q_EMIT changed();
    });
}

bool Displays::ready() const { return !m_config.isNull(); }

QVariantList Displays::outputs() const
{
    QVariantList out;
    if (!m_config)
        return out;
    for (const auto &o : m_config->outputs()) {
        if (!o->isConnected())
            continue;
        QVariantList modes;
        QSet<QString> seen;
        auto list = o->modes().values();
        std::sort(list.begin(), list.end(), [](const KScreen::ModePtr &a, const KScreen::ModePtr &b) {
            if (a->size().width() != b->size().width()) return a->size().width() > b->size().width();
            return a->refreshRate() > b->refreshRate();
        });
        for (const auto &m : list) {
            const QString label = QStringLiteral("%1 × %2 @ %3 Hz").arg(m->size().width()).arg(m->size().height()).arg(qRound(m->refreshRate()));
            if (seen.contains(label)) continue;
            seen.insert(label);
            modes.append(QVariantMap{{QStringLiteral("id"), m->id()}, {QStringLiteral("label"), label}});
        }
        const auto cur = o->currentMode();
        out.append(QVariantMap{{QStringLiteral("id"), o->id()}, {QStringLiteral("name"), o->name()},
                               {QStringLiteral("enabled"), o->isEnabled()}, {QStringLiteral("primary"), o->priority() == 1},
                               {QStringLiteral("scale"), o->scale()}, {QStringLiteral("modes"), modes},
                               {QStringLiteral("currentMode"), cur ? cur->id() : QString()},
                               {QStringLiteral("resolution"), cur ? QStringLiteral("%1 × %2").arg(cur->size().width()).arg(cur->size().height()) : QString()},
                               {QStringLiteral("refresh"), cur ? qRound(cur->refreshRate()) : 0},
                               {QStringLiteral("internal"), o->type() == KScreen::Output::Panel}});
    }
    return out;
}

void Displays::apply()
{
    if (!m_config) return;
    if (!KScreen::Config::canBeApplied(m_config)) {
        Q_EMIT applyFailed(tr("This configuration cannot be applied."));
        reload();
        return;
    }
    auto *op = new KScreen::SetConfigOperation(m_config);
    connect(op, &KScreen::ConfigOperation::finished, this, [this](KScreen::ConfigOperation *o) {
        if (o->hasError()) Q_EMIT applyFailed(o->errorString());
        reload();
    });
}

void Displays::setMode(int id, const QString &modeId)
{
    if (!m_config) return;
    if (auto o = m_config->output(id)) { o->setCurrentModeId(modeId); apply(); }
}

void Displays::setScale(int id, qreal scale)
{
    if (!m_config) return;
    if (auto o = m_config->output(id)) { o->setScale(scale); apply(); }
}

void Displays::setEnabled(int id, bool enabled)
{
    if (!m_config) return;
    if (auto o = m_config->output(id)) { o->setEnabled(enabled); apply(); }
}

// ---------------------------------------------------------------- InputDevices
static const QString kKWin = QStringLiteral("org.kde.KWin");

InputDevices::InputDevices(QObject *parent) : QObject(parent) { reload(); }

void InputDevices::reload()
{
    m_devices.clear();
    auto bus = QDBusConnection::sessionBus();
    const QStringList names = dbusProp(bus, kKWin, QStringLiteral("/org/kde/KWin/InputDevice"),
                                       QStringLiteral("org.kde.KWin.InputDeviceManager"), QStringLiteral("devicesSysNames")).toStringList();
    for (const QString &n : names) {
        const QVariantMap p = dbusAll(bus, kKWin, QStringLiteral("/org/kde/KWin/InputDevice/") + n, QStringLiteral("org.kde.KWin.InputDevice"));
        if (!p.value(QStringLiteral("pointer")).toBool() && !p.value(QStringLiteral("touchpad")).toBool())
            continue;
        QVariantMap d = p;
        d.insert(QStringLiteral("sysName"), n);
        m_devices.append(d);
    }
    Q_EMIT changed();
}

void InputDevices::set(const QString &sysName, const QString &property, const QVariant &value)
{
    QDBusMessage m = QDBusMessage::createMethodCall(kKWin, QStringLiteral("/org/kde/KWin/InputDevice/") + sysName,
                                                    QStringLiteral("org.freedesktop.DBus.Properties"), QStringLiteral("Set"));
    m << QStringLiteral("org.kde.KWin.InputDevice") << property << QVariant::fromValue(QDBusVariant(value));
    QDBusConnection::sessionBus().call(m, QDBus::Block, 2000);
    reload();
}

// ---------------------------------------------------------------- SystemInfo
static QString readFirst(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? QString::fromUtf8(f.readAll()).trimmed() : QString();
}

QString SystemInfo::lookupPciName(const QString &vendor, const QString &device)
{
    QFile f(QStringLiteral("/usr/share/hwdata/pci.ids"));
    if (!f.open(QIODevice::ReadOnly))
        return {};
    QTextStream ts(&f);
    QString vendorName;
    bool inVendor = false;
    while (!ts.atEnd()) {
        const QString line = ts.readLine();
        if (line.startsWith(QLatin1Char('#')) || line.isEmpty()) continue;
        if (!line.startsWith(QLatin1Char('\t'))) {
            inVendor = line.startsWith(vendor, Qt::CaseInsensitive);
            if (inVendor) vendorName = line.mid(6).trimmed();
            else if (!vendorName.isEmpty()) break;
        } else if (inVendor && !line.startsWith(QLatin1String("\t\t")) && line.mid(1).startsWith(device, Qt::CaseInsensitive)) {
            return vendorName + QLatin1Char(' ') + line.mid(6).trimmed();
        }
    }
    return vendorName;
}

QString SystemInfo::gpuName()
{
    QStringList names;
    const QDir drm(QStringLiteral("/sys/class/drm"));
    for (const QString &card : drm.entryList({QStringLiteral("card?")}, QDir::Dirs | QDir::System)) {
        const QString dev = drm.filePath(card) + QStringLiteral("/device/");
        QString v = readFirst(dev + QStringLiteral("vendor")).remove(QStringLiteral("0x"));
        QString d = readFirst(dev + QStringLiteral("device")).remove(QStringLiteral("0x"));
        if (v.isEmpty()) continue;
        const QString n = lookupPciName(v, d);
        if (!n.isEmpty() && !names.contains(n)) names << n;
    }
    return names.isEmpty() ? QObject::tr("Software rendering") : names.join(QStringLiteral(", "));
}

SystemInfo::SystemInfo(QObject *parent) : QObject(parent)
{
    QString cpu;
    int cores = 0;
    QFile ci(QStringLiteral("/proc/cpuinfo"));
    if (ci.open(QIODevice::ReadOnly)) {
        for (const QByteArray &l : ci.readAll().split('\n')) {
            if (l.startsWith("model name") && cpu.isEmpty()) cpu = QString::fromUtf8(l.mid(l.indexOf(':') + 1)).trimmed();
            if (l.startsWith("processor")) ++cores;
        }
    }
    qint64 mem = 0;
    QFile mi(QStringLiteral("/proc/meminfo"));
    if (mi.open(QIODevice::ReadOnly))
        mem = QString::fromUtf8(mi.readLine()).simplified().split(QLatin1Char(' ')).value(1).toLongLong() * 1024;
    QString os = QSysInfo::prettyProductName();
    QFile lr(QStringLiteral("/etc/os-release"));
    QString version = QStringLiteral(LUMEN_VERSION);
    if (lr.open(QIODevice::ReadOnly)) {
        for (const QByteArray &l : lr.readAll().split('\n'))
            if (l.startsWith("LUMEN_VERSION=")) version = QString::fromUtf8(l.mid(14)).remove(QLatin1Char('"'));
    }
    const QStorageInfo root(QStringLiteral("/"));
    m_facts = {{QStringLiteral("os"), os}, {QStringLiteral("lumenVersion"), version},
               {QStringLiteral("kernel"), QSysInfo::kernelVersion()}, {QStringLiteral("cpu"), cpu.isEmpty() ? QSysInfo::currentCpuArchitecture() : cpu},
               {QStringLiteral("cores"), cores}, {QStringLiteral("memory"), mem}, {QStringLiteral("gpu"), gpuName()},
               {QStringLiteral("diskTotal"), root.bytesTotal()}, {QStringLiteral("diskFree"), root.bytesAvailable()},
               {QStringLiteral("model"), readFirst(QStringLiteral("/sys/class/dmi/id/product_name"))},
               {QStringLiteral("vendor"), readFirst(QStringLiteral("/sys/class/dmi/id/sys_vendor"))}};
}

QString SystemInfo::hostname() const
{
    const QVariant v = dbusProp(QDBusConnection::systemBus(), QStringLiteral("org.freedesktop.hostname1"), QStringLiteral("/org/freedesktop/hostname1"),
                                QStringLiteral("org.freedesktop.hostname1"), QStringLiteral("PrettyHostname"));
    return v.toString().isEmpty() ? QSysInfo::machineHostName() : v.toString();
}

void SystemInfo::setHostname(const QString &name)
{
    const QString pretty = name.trimmed();
    if (pretty.isEmpty()) return;
    QString stat = pretty.toLower();
    stat.replace(QRegularExpression(QStringLiteral("[^a-z0-9-]+")), QStringLiteral("-"));
    stat = stat.left(63);
    auto bus = QDBusConnection::systemBus();
    for (const auto &[method, value] : {std::pair{QStringLiteral("SetPrettyHostname"), pretty}, std::pair{QStringLiteral("SetStaticHostname"), stat}}) {
        QDBusMessage m = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.hostname1"), QStringLiteral("/org/freedesktop/hostname1"),
                                                        QStringLiteral("org.freedesktop.hostname1"), method);
        m << value << true; // interactive → polkit authentication
        const QDBusMessage r = bus.call(m, QDBus::Block, 60000);
        if (r.type() == QDBusMessage::ErrorMessage) { Q_EMIT failed(r.errorMessage()); return; }
    }
    Q_EMIT changed();
}

// ---------------------------------------------------------------- Accounts
static const QString kAcc = QStringLiteral("org.freedesktop.Accounts");

Accounts::Accounts(QObject *parent) : QObject(parent) { reload(); }

void Accounts::reload()
{
    auto bus = QDBusConnection::systemBus();
    QDBusInterface mgr(kAcc, QStringLiteral("/org/freedesktop/Accounts"), kAcc, bus);
    QDBusReply<QDBusObjectPath> me = mgr.call(QStringLiteral("FindUserById"), qint64(::getuid()));
    m_path = me.isValid() ? me.value().path() : QString();
    m_user = m_path.isEmpty() ? QVariantMap() : dbusAll(bus, kAcc, m_path, kAcc + QStringLiteral(".User"));
    m_users.clear();
    QDBusReply<QList<QDBusObjectPath>> list = mgr.call(QStringLiteral("ListCachedUsers"));
    if (list.isValid()) {
        for (const auto &p : list.value()) {
            const QVariantMap u = dbusAll(bus, kAcc, p.path(), kAcc + QStringLiteral(".User"));
            m_users.append(QVariantMap{{QStringLiteral("userName"), u.value(QStringLiteral("UserName"))},
                                       {QStringLiteral("realName"), u.value(QStringLiteral("RealName"))},
                                       {QStringLiteral("iconFile"), u.value(QStringLiteral("IconFile"))},
                                       {QStringLiteral("admin"), u.value(QStringLiteral("AccountType")).toInt() == 1}});
        }
    }
    Q_EMIT changed();
}

void Accounts::call(const QString &method, const QVariantList &args)
{
    if (m_path.isEmpty()) { Q_EMIT failed(tr("AccountsService is not available")); return; }
    QDBusMessage m = QDBusMessage::createMethodCall(kAcc, m_path, kAcc + QStringLiteral(".User"), method);
    m.setArguments(args);
    m.setInteractiveAuthorizationAllowed(true);
    auto *w = new QDBusPendingCallWatcher(QDBusConnection::systemBus().asyncCall(m, 120000), this);
    connect(w, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *w) {
        if (w->isError()) Q_EMIT failed(w->error().message());
        w->deleteLater();
        reload();
    });
}

void Accounts::setRealName(const QString &name) { call(QStringLiteral("SetRealName"), {name}); }
void Accounts::setIconFile(const QString &path) { call(QStringLiteral("SetIconFile"), {QUrl(path).isLocalFile() ? QUrl(path).toLocalFile() : path}); }

QString Accounts::hashPassword(const QString &password)
{
    char saltbuf[CRYPT_GENSALT_OUTPUT_SIZE];
    if (!crypt_gensalt_rn("$6$", 0, nullptr, 0, saltbuf, sizeof saltbuf))
        return {};
    crypt_data data{};
    const char *h = crypt_r(password.toUtf8().constData(), saltbuf, &data);
    return (h && h[0] != '*') ? QString::fromLatin1(h) : QString();
}

void Accounts::setPassword(const QString &password, const QString &hint)
{
    const QString hashed = hashPassword(password);
    if (hashed.isEmpty()) { Q_EMIT failed(tr("Could not hash the password")); return; }
    call(QStringLiteral("SetPassword"), {hashed, hint});
}

// ---------------------------------------------------------------- Firewall
Firewall::Firewall(QObject *parent) : QObject(parent) { refresh(); }

void Firewall::refresh()
{
    auto bus = QDBusConnection::systemBus();
    const QVariant unitPath = [&] {
        QDBusMessage m = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.systemd1"), QStringLiteral("/org/freedesktop/systemd1"),
                                                        QStringLiteral("org.freedesktop.systemd1.Manager"), QStringLiteral("LoadUnit"));
        m << QStringLiteral("firewalld.service");
        const QDBusMessage r = bus.call(m, QDBus::Block, 2000);
        return r.type() == QDBusMessage::ReplyMessage ? QVariant(r.arguments().value(0).value<QDBusObjectPath>().path()) : QVariant();
    }();
    m_available = false; m_running = false; m_zone.clear();
    if (unitPath.isValid()) {
        const QString load = dbusProp(bus, QStringLiteral("org.freedesktop.systemd1"), unitPath.toString(),
                                      QStringLiteral("org.freedesktop.systemd1.Unit"), QStringLiteral("LoadState")).toString();
        m_available = load == QLatin1String("loaded");
        m_running = dbusProp(bus, QStringLiteral("org.freedesktop.systemd1"), unitPath.toString(),
                             QStringLiteral("org.freedesktop.systemd1.Unit"), QStringLiteral("ActiveState")).toString() == QLatin1String("active");
    }
    if (m_running) {
        QDBusInterface fw(QStringLiteral("org.fedoraproject.FirewallD1"), QStringLiteral("/org/fedoraproject/FirewallD1"),
                          QStringLiteral("org.fedoraproject.FirewallD1"), bus);
        QDBusReply<QString> z = fw.call(QStringLiteral("getDefaultZone"));
        if (z.isValid()) m_zone = z.value();
    }
    Q_EMIT changed();
}

void Firewall::setRunning(bool on)
{
    auto bus = QDBusConnection::systemBus();
    const QString unit = QStringLiteral("firewalld.service");
    QDBusMessage m = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.systemd1"), QStringLiteral("/org/freedesktop/systemd1"),
                                                    QStringLiteral("org.freedesktop.systemd1.Manager"),
                                                    on ? QStringLiteral("EnableUnitFiles") : QStringLiteral("DisableUnitFiles"));
    if (on) m << QStringList{unit} << false << true;
    else m << QStringList{unit} << false;
    m.setInteractiveAuthorizationAllowed(true);
    bus.call(m, QDBus::Block, 120000);
    QDBusMessage s = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.systemd1"), QStringLiteral("/org/freedesktop/systemd1"),
                                                    QStringLiteral("org.freedesktop.systemd1.Manager"), on ? QStringLiteral("StartUnit") : QStringLiteral("StopUnit"));
    s << unit << QStringLiteral("replace");
    s.setInteractiveAuthorizationAllowed(true);
    const QDBusMessage r = bus.call(s, QDBus::Block, 120000);
    if (r.type() == QDBusMessage::ErrorMessage) Q_EMIT failed(r.errorMessage());
    refresh();
}

// ---------------------------------------------------------------- PowerProfiles
PowerProfiles::PowerProfiles(QObject *parent) : QObject(parent) { refresh(); }

void PowerProfiles::refresh()
{
    auto bus = QDBusConnection::systemBus();
    // Newer releases use org.freedesktop.UPower.PowerProfiles.
    for (const auto &[svc, path] : {std::pair{QStringLiteral("org.freedesktop.UPower.PowerProfiles"), QStringLiteral("/org/freedesktop/UPower/PowerProfiles")},
                                    std::pair{QStringLiteral("net.hadess.PowerProfiles"), QStringLiteral("/net/hadess/PowerProfiles")}}) {
        const QVariant active = dbusProp(bus, svc, path, svc, QStringLiteral("ActiveProfile"));
        if (!active.isValid()) continue;
        m_service = svc; m_path = path; m_iface = svc;
        m_active = active.toString();
        m_profiles.clear();
        const QVariant profiles = dbusProp(bus, svc, path, svc, QStringLiteral("Profiles"));
        const QDBusArgument arg = profiles.value<QDBusArgument>();
        arg.beginArray();
        while (!arg.atEnd()) {
            QVariantMap mp; arg >> mp;
            m_profiles << mp.value(QStringLiteral("Profile")).toString();
        }
        arg.endArray();
        break;
    }
    Q_EMIT changed();
}

void PowerProfiles::setActive(const QString &p)
{
    if (m_service.isEmpty() || p == m_active) return;
    QDBusMessage m = QDBusMessage::createMethodCall(m_service, m_path, QStringLiteral("org.freedesktop.DBus.Properties"), QStringLiteral("Set"));
    m << m_iface << QStringLiteral("ActiveProfile") << QVariant::fromValue(QDBusVariant(p));
    m.setInteractiveAuthorizationAllowed(true);
    QDBusConnection::systemBus().call(m, QDBus::Block, 30000);
    refresh();
}

// ---------------------------------------------------------------- XkbLayouts
XkbLayouts::XkbLayouts(QObject *parent) : QObject(parent)
{
    QFile f(QStringLiteral("/usr/share/X11/xkb/rules/evdev.lst"));
    if (f.open(QIODevice::ReadOnly))
        m_all = parse(QString::fromUtf8(f.readAll()));
}

QVariantList XkbLayouts::parse(const QString &text)
{
    QVariantList out;
    bool inLayouts = false;
    for (const QString &line : text.split(QLatin1Char('\n'))) {
        if (line.startsWith(QLatin1Char('!'))) { inLayouts = line.trimmed() == QLatin1String("! layout"); continue; }
        if (!inLayouts || line.trimmed().isEmpty()) continue;
        const QString t = line.trimmed();
        const int sp = t.indexOf(QLatin1Char(' '));
        if (sp < 0) continue;
        out.append(QVariantMap{{QStringLiteral("code"), t.left(sp)}, {QStringLiteral("name"), t.mid(sp).trimmed()}});
    }
    return out;
}

} // namespace Lumen
