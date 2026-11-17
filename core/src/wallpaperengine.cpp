// SPDX-License-Identifier: GPL-3.0-or-later
#include "wallpaperengine.h"
#include "shellsettings.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlEngine>
#include <QStandardPaths>
#include <QTime>

namespace Lumen {

WallpaperEngine::WallpaperEngine(QObject *parent) : QObject(parent)
{
    loadCollection();
    connect(ShellSettings::instance(), &ShellSettings::changed, this, &WallpaperEngine::update);
    m_timer.setInterval(30 * 1000);
    connect(&m_timer, &QTimer::timeout, this, &WallpaperEngine::update);
    m_timer.start();
    update();
}

WallpaperEngine *WallpaperEngine::instance()
{
    static WallpaperEngine *s = new WallpaperEngine;
    return s;
}

WallpaperEngine *WallpaperEngine::create(QQmlEngine *, QJSEngine *)
{
    QQmlEngine::setObjectOwnership(instance(), QQmlEngine::CppOwnership);
    return instance();
}

QString WallpaperEngine::dataDir()
{
    const QString env = qEnvironmentVariable("LUMEN_WALLPAPER_DIR");
    if (!env.isEmpty())
        return env;
    const QString found = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("lumen/wallpapers"),
                                                 QStandardPaths::LocateDirectory);
    return found.isEmpty() ? QStringLiteral("/usr/share/lumen/wallpapers") : found;
}

void WallpaperEngine::loadCollection()
{
    QFile f(dataDir() + QStringLiteral("/collection.json"));
    m_wallpapers.clear();
    m_dynamic.clear();
    if (f.open(QIODevice::ReadOnly)) {
        const QJsonObject root = QJsonDocument::fromJson(f.readAll()).object();
        m_default = root.value(QStringLiteral("default")).toString();
        const QJsonObject wp = root.value(QStringLiteral("wallpapers")).toObject();
        for (auto it = wp.begin(); it != wp.end(); ++it) {
            const QJsonObject o = it.value().toObject();
            const QString path = resolve(it.key());
            if (path.isEmpty()) continue;
            m_wallpapers.append(QVariantMap{{QStringLiteral("name"), it.key()},
                                            {QStringLiteral("title"), o.value(QStringLiteral("title")).toString()},
                                            {QStringLiteral("category"), o.value(QStringLiteral("category")).toString()},
                                            {QStringLiteral("source"), QUrl::fromLocalFile(path)}});
        }
        m_dynamic = root.value(QStringLiteral("dynamic")).toObject().toVariantMap();
    }
    Q_EMIT collectionChanged();
}

QStringList WallpaperEngine::dynamicSets() const { return m_dynamic.keys(); }

QString WallpaperEngine::resolve(const QString &nameOrPath) const
{
    if (nameOrPath.isEmpty()) return {};
    if (QFileInfo(nameOrPath).isAbsolute())
        return QFileInfo::exists(nameOrPath) ? nameOrPath : QString();
    for (const char *ext : {".jpg", ".png", ".webp"}) {
        const QString p = dataDir() + QLatin1Char('/') + nameOrPath + QLatin1String(ext);
        if (QFileInfo::exists(p)) return p;
    }
    return {};
}

QString WallpaperEngine::dynamicImageAt(const QString &set, int minutes) const
{
    const QVariantList steps = m_dynamic.value(set).toList();
    QString chosen;
    int bestFrom = -1, latestFrom = -1;
    QString latest;
    for (const QVariant &v : steps) {
        const QVariantMap m = v.toMap();
        const QTime t = QTime::fromString(m.value(QStringLiteral("from")).toString(), QStringLiteral("HH:mm"));
        const int from = t.hour() * 60 + t.minute();
        if (from <= minutes && from > bestFrom) { bestFrom = from; chosen = m.value(QStringLiteral("wallpaper")).toString(); }
        if (from > latestFrom) { latestFrom = from; latest = m.value(QStringLiteral("wallpaper")).toString(); }
    }
    return chosen.isEmpty() ? latest : chosen; // before the first step: last step of the previous day
}

void WallpaperEngine::setStatic(const QString &p)
{
    ShellSettings::instance()->setDynamicWallpaper(QString());
    ShellSettings::instance()->setWallpaper(p);
}

void WallpaperEngine::setDynamic(const QString &set)
{
    ShellSettings::instance()->setDynamicWallpaper(set);
}

void WallpaperEngine::update()
{
    auto *s = ShellSettings::instance();
    QString path;
    if (!s->dynamicWallpaper().isEmpty() && m_dynamic.contains(s->dynamicWallpaper())) {
        const QTime now = QTime::currentTime();
        path = resolve(dynamicImageAt(s->dynamicWallpaper(), now.hour() * 60 + now.minute()));
    }
    if (path.isEmpty()) path = resolve(s->wallpaper());
    if (path.isEmpty()) path = resolve(m_default);
    if (path.isEmpty()) {
        // Last resort: the first image in the wallpaper directory, so a broken
        // collection or wiped settings never degrade to a black desktop.
        const QDir dir(dataDir());
        const QStringList imgs = dir.entryList({QStringLiteral("*.png"), QStringLiteral("*.jpg"), QStringLiteral("*.webp")},
                                               QDir::Files, QDir::Name);
        if (!imgs.isEmpty())
            path = dir.filePath(imgs.first());
    }
    const QUrl url = path.isEmpty() ? QUrl() : QUrl::fromLocalFile(path);
    if (url == m_source) return;
    m_source = url;
    // Luminance analysis on a small decode (cheap: ~64×36 px).
    QImageReader r(path);
    r.setScaledSize(QSize(64, 36));
    const QImage img = r.read().convertToFormat(QImage::Format_RGB32);
    if (!img.isNull()) {
        auto lum = [&](int y0, int y1) {
            double sum = 0; int n = 0;
            for (int y = y0; y < y1; ++y)
                for (int x = 0; x < img.width(); ++x) {
                    const QColor c = img.pixelColor(x, y);
                    sum += 0.2126 * c.redF() + 0.7152 * c.greenF() + 0.0722 * c.blueF(); ++n;
                }
            return n ? sum / n : 0.3;
        };
        m_topLum = lum(0, 3);
        m_lum = lum(0, img.height());
    }
    Q_EMIT sourceChanged();
}
}
