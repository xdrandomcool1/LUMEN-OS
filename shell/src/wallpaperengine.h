// SPDX-License-Identifier: GPL-3.0-or-later
// Wallpaper selection: static images and time-of-day dynamic sets.
#pragma once
#include <QColor>
#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QVariantList>
#include <QQmlEngine>
#include <QtQml/qqmlregistration.h>

namespace Lumen {

/*!
 * Wallpapers live in <data>/lumen/wallpapers/ next to collection.json, which
 * describes titles, categories and dynamic schedules ("from" times). When a
 * dynamic set is selected the engine switches images at the scheduled times;
 * the shell cross-fades between them. The average luminance of the top strip
 * is exposed so the transparent menu bar can pick legible text colour.
 */
class WallpaperEngine : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QUrl source READ source NOTIFY sourceChanged)
    Q_PROPERTY(qreal topLuminance READ topLuminance NOTIFY sourceChanged)
    Q_PROPERTY(qreal luminance READ luminance NOTIFY sourceChanged)
    Q_PROPERTY(QVariantList wallpapers READ wallpapers NOTIFY collectionChanged)
    Q_PROPERTY(QStringList dynamicSets READ dynamicSets NOTIFY collectionChanged)

public:
    static WallpaperEngine *instance();
    static WallpaperEngine *create(QQmlEngine *, QJSEngine *);
    QUrl source() const { return m_source; }
    qreal topLuminance() const { return m_topLum; }
    qreal luminance() const { return m_lum; }
    QVariantList wallpapers() const { return m_wallpapers; }
    QStringList dynamicSets() const;

    //! Resolves the image a dynamic set shows at \a minutesOfDay.
    Q_INVOKABLE QString dynamicImageAt(const QString &set, int minutesOfDay) const;
    Q_INVOKABLE void setStatic(const QString &pathOrName);
    Q_INVOKABLE void setDynamic(const QString &set);
    static QString dataDir();

Q_SIGNALS:
    void sourceChanged();
    void collectionChanged();

private:
    explicit WallpaperEngine(QObject *parent = nullptr);
    void loadCollection();
    void update();
    QString resolve(const QString &nameOrPath) const;

    QUrl m_source;
    qreal m_topLum = 0.3;
    qreal m_lum = 0.3;
    QVariantList m_wallpapers;
    QVariantMap m_dynamic; // set -> [{from, wallpaper}]
    QString m_default;
    QTimer m_timer;
};
}
