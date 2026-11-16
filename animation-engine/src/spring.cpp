// SPDX-License-Identifier: GPL-3.0-or-later
#include "spring.h"
#include "motion.h"

#include <QtMath>

namespace Lumen {

// Fixed integration step: semi-implicit Euler is stable for every preset at
// this step size and gives identical trajectories at any display refresh rate.
static constexpr qreal kStep = 1.0 / 480.0;
// Never simulate more than this in one frame (e.g. after a stall or suspend):
// jump towards the target instead of replaying seconds of physics.
static constexpr qreal kMaxFrame = 0.064;

SpringTicker::SpringTicker(Spring *spring)
    : QAbstractAnimation(spring)
    , m_spring(spring)
{
}

void SpringTicker::updateState(State newState, State)
{
    if (newState == Running)
        m_lastTime = currentTime();
}

void SpringTicker::updateCurrentTime(int currentTime)
{
    const int delta = currentTime - m_lastTime;
    m_lastTime = currentTime;
    if (delta <= 0)
        return;
    const qreal scale = Motion::instance()->animationScale();
    const qreal dt = qMin(delta / 1000.0, kMaxFrame) / qMax(scale, 0.01);
    if (m_spring->step(dt))
        stop();
}

void Spring::presetParameters(Preset p, qreal &stiffness, qreal &damping, qreal &mass)
{
    mass = 1.0;
    switch (p) {
    case Snappy:   stiffness = 520; damping = 42; break; // ζ ≈ 0.92
    case Smooth:   stiffness = 300; damping = 34; break; // ζ ≈ 0.98
    case Bouncy:   stiffness = 380; damping = 22; break; // ζ ≈ 0.56
    case Gentle:   stiffness = 170; damping = 26; break; // ζ ≈ 1.0
    case Critical: stiffness = 400; damping = 40; break; // ζ = 1.0
    case Custom:   break;
    }
}

Spring::Spring(QObject *parent)
    : QObject(parent)
    , m_ticker(new SpringTicker(this))
{
    presetParameters(m_preset, m_stiffness, m_damping, m_mass);
}

Spring::~Spring() = default;

bool Spring::animationsAllowed() const
{
    if (!m_enabled)
        return false;
    if (m_respectReducedMotion && Motion::instance()->reduceMotion())
        return false;
    return true;
}

void Spring::setTo(qreal to)
{
    m_toSet = true;
    if (qFuzzyCompare(m_to, to))
        return;
    m_to = to;
    Q_EMIT toChanged();
    if (!m_complete)
        return;
    if (!animationsAllowed()) {
        snap(to);
        return;
    }
    start();
}

void Spring::setValue(qreal value)
{
    if (!m_complete)
        m_valueSet = true;
    if (qFuzzyCompare(m_value, value))
        return;
    m_value = value;
    writeTarget();
    Q_EMIT valueChanged();
    if (m_complete && animationsAllowed() && !qFuzzyCompare(m_value, m_to))
        start();
}

void Spring::setVelocity(qreal v)
{
    if (qFuzzyCompare(m_velocity, v))
        return;
    m_velocity = v;
    Q_EMIT velocityChanged();
}

void Spring::setStiffness(qreal k)
{
    k = qMax<qreal>(k, 1.0);
    if (qFuzzyCompare(m_stiffness, k))
        return;
    m_stiffness = k;
    m_preset = Custom;
    Q_EMIT parametersChanged();
}

void Spring::setDamping(qreal c)
{
    c = qMax<qreal>(c, 0.0);
    if (qFuzzyCompare(m_damping, c))
        return;
    m_damping = c;
    m_preset = Custom;
    Q_EMIT parametersChanged();
}

void Spring::setMass(qreal m)
{
    m = qMax<qreal>(m, 0.01);
    if (qFuzzyCompare(m_mass, m))
        return;
    m_mass = m;
    m_preset = Custom;
    Q_EMIT parametersChanged();
}

void Spring::setEpsilon(qreal e)
{
    m_epsilon = qMax<qreal>(e, 1e-6);
    Q_EMIT parametersChanged();
}

void Spring::setPreset(Preset p)
{
    if (m_preset == p)
        return;
    m_preset = p;
    presetParameters(p, m_stiffness, m_damping, m_mass);
    Q_EMIT presetChanged();
    Q_EMIT parametersChanged();
}

void Spring::setEnabled(bool e)
{
    if (m_enabled == e)
        return;
    m_enabled = e;
    Q_EMIT enabledChanged();
    if (!e)
        snap(m_to);
}

bool Spring::isRunning() const
{
    return m_ticker->state() == QAbstractAnimation::Running;
}

void Spring::snap(qreal v)
{
    const bool wasRunning = isRunning();
    m_ticker->stop();
    m_accumulator = 0;
    if (!qFuzzyCompare(m_to, v)) {
        m_to = v;
        Q_EMIT toChanged();
    }
    setVelocity(0);
    if (!qFuzzyCompare(m_value, v)) {
        m_value = v;
        writeTarget();
        Q_EMIT valueChanged();
    }
    if (wasRunning)
        Q_EMIT runningChanged();
}

void Spring::impulse(qreal v)
{
    setVelocity(m_velocity + v);
    if (m_complete && animationsAllowed())
        start();
}

void Spring::start()
{
    if (isRunning())
        return;
    m_accumulator = 0;
    m_ticker->start();
    Q_EMIT runningChanged();
}

bool Spring::step(qreal dt)
{
    m_accumulator += dt;
    qreal x = m_value;
    qreal v = m_velocity;
    while (m_accumulator >= kStep) {
        const qreal force = -m_stiffness * (x - m_to) - m_damping * v;
        v += (force / m_mass) * kStep;
        x += v * kStep;
        m_accumulator -= kStep;
    }

    // Settle when both displacement and velocity are below epsilon, scaled by
    // the size of the motion so that small and large values settle alike.
    const qreal scale = qMax<qreal>(1.0, qAbs(m_to));
    const bool done = qAbs(x - m_to) < m_epsilon * scale && qAbs(v) < m_epsilon * 10 * scale;
    if (done) {
        x = m_to;
        v = 0;
    }

    m_velocity = v;
    Q_EMIT velocityChanged();
    if (!qFuzzyCompare(m_value, x)) {
        m_value = x;
        writeTarget();
        Q_EMIT valueChanged();
    }
    if (done) {
        Q_EMIT runningChanged();
        Q_EMIT settled();
    }
    return done;
}

void Spring::componentComplete()
{
    m_complete = true;
    if (m_property.isValid() && !m_toSet)
        m_to = m_property.read().toReal();
    if (!m_valueSet) {
        // The first value is applied without animation.
        m_value = m_to;
        writeTarget();
        Q_EMIT valueChanged();
    } else {
        // Explicit start value: animate from it (entrance animations).
        writeTarget();
        if (animationsAllowed() && !qFuzzyCompare(m_value, m_to))
            start();
        else
            snap(m_to);
    }
}

void Spring::setTarget(const QQmlProperty &prop)
{
    m_property = prop;
}

void Spring::writeTarget()
{
    if (m_property.isValid())
        m_property.write(m_value);
}

} // namespace Lumen
