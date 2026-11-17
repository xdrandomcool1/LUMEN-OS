// SPDX-License-Identifier: GPL-3.0-or-later
#include "appearance.h"
#include "motion.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QQmlEngine>
#include <QSettings>
#include <QStandardPaths>
#include <QTime>

namespace Lumen {

namespace {
struct Accent { const char *name; const char *light; const char *dark; };
constexpr Accent kAccents[] = {
    {"blue",     "#0A7CFF", "#0A84FF"},
    {"purple",   "#A550D8", "#BF5AF2"},
    {"pink",     "#F2306A", "#FF375F"},
    {"red",      "#F2443A", "#FF453A"},
    {"orange",   "#F59000", "#FF9F0A"},
    {"yellow",   "#E8B800", "#FFD60A"},
    {"green",    "#27B84C", "#30D158"},
    {"graphite", "#8C8C91", "#98989D"},
};

QString modeToString(Appearance::Mode m)
{
    switch (m) {
    case Appearance::Light: return QStringLiteral("light");
    case Appearance::Dark: return QStringLiteral("dark");
    case Appearance::Auto: return QStringLiteral("auto");
    }
    return QStringLiteral("dark");
}

Appearance::Mode modeFromString(const QString &s)
{
    if (s == QLatin1String("light")) return Appearance::Light;
    if (s == QLatin1String("auto")) return Appearance::Auto;
    return Appearance::Dark;
}

bool isNightTime()
{
    const int h = QTime::currentTime().hour();
    return h >= 19 || h < 7;
}
} // namespace

Appearance::Appearance(QObject *parent)
    : QObject(parent)
{
    load();
    applyEnvironmentOverrides();
    watch();
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, [this] {
        if (m_saving)
            return;
        load();
        applyEnvironmentOverrides();
        watch(); // editors often replace the file atomically
        Q_EMIT changed();
    });
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, [this] {
        if (!m_saving && QFileInfo::exists(configPath()) && !m_watcher.files().contains(configPath())) {
            load();
            applyEnvironmentOverrides();
            watch();
            Q_EMIT changed();
        }
    });
    m_autoIsDark = isNightTime();
    m_autoTimer.setInterval(60 * 1000);
    connect(&m_autoTimer, &QTimer::timeout, this, [this] {
        const bool night = isNightTime();
        if (night != m_autoIsDark) {
            m_autoIsDark = night;
            if (m_mode == Auto)
                Q_EMIT changed();
        }
    });
    m_autoTimer.start();
    Motion::instance()->setReduceMotion(m_reduceMotion || Motion::instance()->reduceMotion());
}

Appearance *Appearance::instance()
{
    static Appearance *s = new Appearance;
    return s;
}

Appearance *Appearance::create(QQmlEngine *, QJSEngine *)
{
    auto *a = instance();
    QQmlEngine::setObjectOwnership(a, QQmlEngine::CppOwnership);
    return a;
}

QString Appearance::configPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
        + QStringLiteral("/lumen/lumenrc");
}

void Appearance::load()
{
    // System defaults (/etc/xdg/lumen/lumenrc) are read first, user file wins.
    const QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation);
    for (auto it = dirs.crbegin(); it != dirs.crend(); ++it) {
        const QString path = *it + QStringLiteral("/lumen/lumenrc");
        if (!QFileInfo::exists(path))
            continue;
        QSettings s(path, QSettings::IniFormat);
        s.beginGroup(QStringLiteral("Appearance"));
        m_mode = modeFromString(s.value(QStringLiteral("mode"), modeToString(m_mode)).toString());
        m_accentName = s.value(QStringLiteral("accent"), m_accentName).toString();
        m_reduceTransparency = s.value(QStringLiteral("reduceTransparency"), m_reduceTransparency).toBool();
        m_increaseContrast = s.value(QStringLiteral("increaseContrast"), m_increaseContrast).toBool();
        m_reduceMotion = s.value(QStringLiteral("reduceMotion"), m_reduceMotion).toBool();
        m_glassIntensity = s.value(QStringLiteral("glassIntensity"), m_glassIntensity).toDouble();
    }
    Motion::instance()->setReduceMotion(m_reduceMotion);
}

void Appearance::applyEnvironmentOverrides()
{
    const QString env = qEnvironmentVariable("LUMEN_APPEARANCE");
    if (!env.isEmpty())
        m_mode = modeFromString(env);
    if (qEnvironmentVariableIsSet("LUMEN_REDUCE_TRANSPARENCY"))
        m_reduceTransparency = qEnvironmentVariableIntValue("LUMEN_REDUCE_TRANSPARENCY") == 1;
}

void Appearance::save()
{
    m_saving = true;
    QDir().mkpath(QFileInfo(configPath()).absolutePath());
    {
        QSettings s(configPath(), QSettings::IniFormat);
        s.beginGroup(QStringLiteral("Appearance"));
        s.setValue(QStringLiteral("mode"), modeToString(m_mode));
        s.setValue(QStringLiteral("accent"), m_accentName);
        s.setValue(QStringLiteral("reduceTransparency"), m_reduceTransparency);
        s.setValue(QStringLiteral("increaseContrast"), m_increaseContrast);
        s.setValue(QStringLiteral("reduceMotion"), m_reduceMotion);
        s.setValue(QStringLiteral("glassIntensity"), m_glassIntensity);
        s.sync();
    }
    watch();
    // Let the watcher event generated by our own write pass before re-enabling.
    QTimer::singleShot(150, this, [this] { m_saving = false; });
}

void Appearance::watch()
{
    const QString path = configPath();
    const QString dir = QFileInfo(path).absolutePath();
    QDir().mkpath(dir);
    if (!m_watcher.directories().contains(dir))
        m_watcher.addPath(dir);
    if (QFileInfo::exists(path) && !m_watcher.files().contains(path))
        m_watcher.addPath(path);
}

void Appearance::setMode(Mode m)
{
    if (m_mode == m)
        return;
    m_mode = m;
    save();
    Q_EMIT changed();
}

bool Appearance::dark() const
{
    if (m_mode == Auto)
        return m_autoIsDark;
    return m_mode == Dark;
}

void Appearance::setAccentName(const QString &name)
{
    if (m_accentName == name)
        return;
    m_accentName = name;
    save();
    Q_EMIT changed();
}

QColor Appearance::accentColor(const QString &name) const
{
    for (const auto &a : kAccents) {
        if (name == QLatin1String(a.name))
            return QColor(QLatin1String(dark() ? a.dark : a.light));
    }
    return QColor(QLatin1String(dark() ? kAccents[0].dark : kAccents[0].light));
}

QColor Appearance::accent() const
{
    return accentColor(m_accentName);
}

QStringList Appearance::accentNames() const
{
    QStringList names;
    for (const auto &a : kAccents)
        names << QLatin1String(a.name);
    return names;
}

void Appearance::setReduceTransparency(bool v)
{
    if (m_reduceTransparency == v)
        return;
    m_reduceTransparency = v;
    save();
    Q_EMIT changed();
}

void Appearance::setIncreaseContrast(bool v)
{
    if (m_increaseContrast == v)
        return;
    m_increaseContrast = v;
    save();
    Q_EMIT changed();
}

void Appearance::setReduceMotion(bool v)
{
    if (m_reduceMotion == v)
        return;
    m_reduceMotion = v;
    Motion::instance()->setReduceMotion(v);
    save();
    Q_EMIT changed();
}

void Appearance::setGlassIntensity(qreal v)
{
    v = qBound<qreal>(0.0, v, 1.5);
    if (qFuzzyCompare(m_glassIntensity, v))
        return;
    m_glassIntensity = v;
    save();
    Q_EMIT changed();
}

} // namespace Lumen
