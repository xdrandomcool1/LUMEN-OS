// SPDX-License-Identifier: GPL-3.0-or-later
// Lumen.Design — system-wide appearance state shared by every LUMEN process.
#pragma once

#include <QColor>
#include <QFileSystemWatcher>
#include <QObject>
#include <QTimer>
#include <QtQml/qqmlregistration.h>

class QQmlEngine;
class QJSEngine;

namespace Lumen {

/*!
 * Appearance settings live in ~/.config/lumen/lumenrc ([Appearance] group).
 * Every process watches the file, so flipping Dark Mode in Control Center
 * restyles the shell and all running Lumen apps within one frame.
 */
class Appearance : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY changed)
    Q_PROPERTY(bool dark READ dark NOTIFY changed)
    Q_PROPERTY(QString accentName READ accentName WRITE setAccentName NOTIFY changed)
    Q_PROPERTY(QColor accent READ accent NOTIFY changed)
    Q_PROPERTY(bool reduceTransparency READ reduceTransparency WRITE setReduceTransparency NOTIFY changed)
    Q_PROPERTY(bool increaseContrast READ increaseContrast WRITE setIncreaseContrast NOTIFY changed)
    Q_PROPERTY(bool reduceMotion READ reduceMotion WRITE setReduceMotion NOTIFY changed)
    Q_PROPERTY(qreal glassIntensity READ glassIntensity WRITE setGlassIntensity NOTIFY changed)
    Q_PROPERTY(QStringList accentNames READ accentNames CONSTANT)

public:
    enum Mode { Light, Dark, Auto };
    Q_ENUM(Mode)

    static Appearance *instance();
    static Appearance *create(QQmlEngine *, QJSEngine *);

    Mode mode() const { return m_mode; }
    void setMode(Mode m);
    bool dark() const;
    QString accentName() const { return m_accentName; }
    void setAccentName(const QString &name);
    QColor accent() const;
    Q_INVOKABLE QColor accentColor(const QString &name) const;
    QStringList accentNames() const;
    bool reduceTransparency() const { return m_reduceTransparency; }
    void setReduceTransparency(bool v);
    bool increaseContrast() const { return m_increaseContrast; }
    void setIncreaseContrast(bool v);
    bool reduceMotion() const { return m_reduceMotion; }
    void setReduceMotion(bool v);
    qreal glassIntensity() const { return m_glassIntensity; }
    void setGlassIntensity(qreal v);

    static QString configPath();

Q_SIGNALS:
    void changed();

private:
    explicit Appearance(QObject *parent = nullptr);
    void load();
    void save();
    void watch();
    void applyEnvironmentOverrides();

    QFileSystemWatcher m_watcher;
    QTimer m_autoTimer; // re-evaluates Auto mode (sunset/sunrise) every minute
    Mode m_mode = Dark;
    QString m_accentName = QStringLiteral("blue");
    bool m_reduceTransparency = false;
    bool m_increaseContrast = false;
    bool m_reduceMotion = false;
    qreal m_glassIntensity = 1.0;
    bool m_autoIsDark = true;
    bool m_saving = false;
};

} // namespace Lumen
