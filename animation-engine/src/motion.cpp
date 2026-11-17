// SPDX-License-Identifier: GPL-3.0-or-later
#include "motion.h"

#include <QCoreApplication>
#include <QQmlEngine>
#include <QtMath>

namespace Lumen {

Motion::Motion(QObject *parent)
    : QObject(parent)
{
    // Environment overrides are useful for screenshots/tests and for users who
    // configure accessibility before the settings daemon is up.
    m_reduceMotion = qEnvironmentVariableIntValue("LUMEN_REDUCE_MOTION") == 1;
    bool ok = false;
    const qreal s = qEnvironmentVariable("LUMEN_ANIMATION_SCALE").toDouble(&ok);
    if (ok && s > 0)
        m_scale = s;
}

Motion *Motion::instance()
{
    static Motion *s_instance = [] {
        auto *m = new Motion;
        if (QCoreApplication::instance())
            m->moveToThread(QCoreApplication::instance()->thread());
        return m;
    }();
    return s_instance;
}

Motion *Motion::create(QQmlEngine *, QJSEngine *)
{
    auto *m = instance();
    QQmlEngine::setObjectOwnership(m, QQmlEngine::CppOwnership);
    return m;
}

void Motion::setReduceMotion(bool r)
{
    if (m_reduceMotion == r)
        return;
    m_reduceMotion = r;
    Q_EMIT reduceMotionChanged();
    Q_EMIT timingChanged();
}

void Motion::setAnimationScale(qreal s)
{
    s = qBound<qreal>(0.05, s, 20.0);
    if (qFuzzyCompare(m_scale, s))
        return;
    m_scale = s;
    Q_EMIT animationScaleChanged();
    Q_EMIT timingChanged();
}

int Motion::scaled(int ms) const
{
    if (m_reduceMotion)
        return qMin(ms, 120);
    return qRound(ms * m_scale);
}

qreal Motion::rubberBand(qreal offset, qreal dimension, qreal c)
{
    if (dimension <= 0)
        return 0;
    const qreal sign = offset < 0 ? -1 : 1;
    const qreal x = qAbs(offset);
    return sign * (1.0 - (1.0 / ((x * c / dimension) + 1.0))) * dimension;
}

} // namespace Lumen
