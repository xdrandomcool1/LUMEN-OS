// SPDX-License-Identifier: GPL-3.0-or-later
// Lumen.Motion — global motion tokens and accessibility switches.
#pragma once

#include <QEasingCurve>
#include <QObject>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

class QQmlEngine;
class QJSEngine;

namespace Lumen {

/*!
 * Single source of truth for durations and curves used by every LUMEN UI
 * process. Durations collapse to short cross-fades when reduceMotion is on.
 * The same values are exported to compositor/kwin-scripts/lumen-motion.js at
 * build time so KWin effects use identical timing.
 */
class Motion : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool reduceMotion READ reduceMotion WRITE setReduceMotion NOTIFY reduceMotionChanged)
    Q_PROPERTY(qreal animationScale READ animationScale WRITE setAnimationScale NOTIFY animationScaleChanged)

    Q_PROPERTY(int instant READ instant NOTIFY timingChanged)
    Q_PROPERTY(int fast READ fast NOTIFY timingChanged)
    Q_PROPERTY(int normal READ normal NOTIFY timingChanged)
    Q_PROPERTY(int slow READ slow NOTIFY timingChanged)
    Q_PROPERTY(int windowOpen READ windowOpen NOTIFY timingChanged)
    Q_PROPERTY(int windowClose READ windowClose NOTIFY timingChanged)
    Q_PROPERTY(int minimize READ minimize NOTIFY timingChanged)
    Q_PROPERTY(int tooltipDelay READ tooltipDelay CONSTANT)

    // Cubic bezier control points in QML's Easing.Bezier list format.
    Q_PROPERTY(QVariantList standard READ standard CONSTANT)
    Q_PROPERTY(QVariantList emphasized READ emphasized CONSTANT)
    Q_PROPERTY(QVariantList exit READ exit CONSTANT)
    Q_PROPERTY(QVariantList decelerate READ decelerate CONSTANT)

public:
    static Motion *instance();
    static Motion *create(QQmlEngine *, QJSEngine *);

    bool reduceMotion() const { return m_reduceMotion; }
    void setReduceMotion(bool r);
    qreal animationScale() const { return m_scale; }
    void setAnimationScale(qreal s);

    int instant() const { return scaled(90); }
    int fast() const { return scaled(160); }
    int normal() const { return scaled(240); }
    int slow() const { return scaled(360); }
    int windowOpen() const { return scaled(280); }
    int windowClose() const { return scaled(200); }
    int minimize() const { return scaled(420); }
    int tooltipDelay() const { return 600; }

    QVariantList standard() const { return {0.25, 0.1, 0.25, 1.0, 1.0, 1.0}; }
    QVariantList emphasized() const { return {0.2, 0.0, 0.0, 1.0, 1.0, 1.0}; }
    QVariantList exit() const { return {0.4, 0.0, 1.0, 1.0, 1.0, 1.0}; }
    QVariantList decelerate() const { return {0.0, 0.0, 0.2, 1.0, 1.0, 1.0}; }

    //! Duration helper for ad-hoc values: Motion.duration(500)
    Q_INVOKABLE int duration(int ms) const { return scaled(ms); }
    //! Rubber-band resistance used for overscroll and drag past bounds.
    Q_INVOKABLE static qreal rubberBand(qreal offset, qreal dimension, qreal coefficient = 0.55);

Q_SIGNALS:
    void reduceMotionChanged();
    void animationScaleChanged();
    void timingChanged();

private:
    explicit Motion(QObject *parent = nullptr);
    int scaled(int ms) const;

    bool m_reduceMotion = false;
    qreal m_scale = 1.0;
};

} // namespace Lumen
