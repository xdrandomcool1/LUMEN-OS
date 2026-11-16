// SPDX-License-Identifier: GPL-3.0-or-later
// Lumen.Motion — physically based spring animation.
#pragma once

#include <QAbstractAnimation>
#include <QObject>
#include <QQmlParserStatus>
#include <QQmlProperty>
#include <QQmlPropertyValueSource>
#include <QtQml/qqmlregistration.h>

namespace Lumen {

class Spring;

// Drives a Spring from Qt's animation driver. With the threaded/basic render
// loops of Qt Quick the driver advances once per presented frame, so springs
// update in lock-step with the display's refresh rate (60/120/144/240 Hz).
class SpringTicker final : public QAbstractAnimation
{
public:
    explicit SpringTicker(Spring *spring);
    int duration() const override { return -1; }

protected:
    void updateCurrentTime(int currentTime) override;
    void updateState(State newState, State oldState) override;

private:
    Spring *m_spring;
    int m_lastTime = 0;
};

/*!
 * A damped harmonic oscillator that animates `value` towards `to`.
 *
 * Springs are interruptible: changing `to` mid-flight keeps the current
 * velocity, which is what makes gesture-driven and rapidly re-targeted motion
 * (Dock magnification, panel drags) feel continuous.
 *
 *   Spring { id: s; to: hovered ? 1.2 : 1.0; preset: Spring.Snappy }
 *   scale: s.value
 * or
 *   Spring on scale { to: hovered ? 1.2 : 1.0 }
 */
class Spring : public QObject, public QQmlParserStatus, public QQmlPropertyValueSource
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus QQmlPropertyValueSource)
    QML_ELEMENT

    Q_PROPERTY(qreal to READ to WRITE setTo NOTIFY toChanged)
    Q_PROPERTY(qreal value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(qreal velocity READ velocity WRITE setVelocity NOTIFY velocityChanged)
    Q_PROPERTY(qreal stiffness READ stiffness WRITE setStiffness NOTIFY parametersChanged)
    Q_PROPERTY(qreal damping READ damping WRITE setDamping NOTIFY parametersChanged)
    Q_PROPERTY(qreal mass READ mass WRITE setMass NOTIFY parametersChanged)
    Q_PROPERTY(qreal epsilon READ epsilon WRITE setEpsilon NOTIFY parametersChanged)
    Q_PROPERTY(Preset preset READ preset WRITE setPreset NOTIFY presetChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(bool respectReducedMotion MEMBER m_respectReducedMotion NOTIFY parametersChanged)

public:
    enum Preset {
        Custom,
        Snappy,   // hover, presses, toggles
        Smooth,   // panels, menus
        Bouncy,   // launch bounce, playful confirmations
        Gentle,   // window open, overview
        Critical  // critically damped, no overshoot
    };
    Q_ENUM(Preset)

    explicit Spring(QObject *parent = nullptr);
    ~Spring() override;

    qreal to() const { return m_to; }
    void setTo(qreal to);
    qreal value() const { return m_value; }
    void setValue(qreal value);
    qreal velocity() const { return m_velocity; }
    void setVelocity(qreal v);
    qreal stiffness() const { return m_stiffness; }
    void setStiffness(qreal k);
    qreal damping() const { return m_damping; }
    void setDamping(qreal c);
    qreal mass() const { return m_mass; }
    void setMass(qreal m);
    qreal epsilon() const { return m_epsilon; }
    void setEpsilon(qreal e);
    Preset preset() const { return m_preset; }
    void setPreset(Preset p);
    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool e);
    bool isRunning() const;

    //! Jump to \a v with zero velocity (no animation).
    Q_INVOKABLE void snap(qreal v);
    //! Add instantaneous velocity (e.g. from a fling gesture).
    Q_INVOKABLE void impulse(qreal v);

    //! Advance the simulation by \a dt seconds. Returns true when settled.
    bool step(qreal dt);

    static void presetParameters(Preset p, qreal &stiffness, qreal &damping, qreal &mass);

    // QQmlParserStatus
    void classBegin() override {}
    void componentComplete() override;
    // QQmlPropertyValueSource
    void setTarget(const QQmlProperty &prop) override;

Q_SIGNALS:
    void toChanged();
    void valueChanged();
    void velocityChanged();
    void parametersChanged();
    void presetChanged();
    void enabledChanged();
    void runningChanged();
    void settled();

private:
    void start();
    void writeTarget();
    bool animationsAllowed() const;

    SpringTicker *m_ticker;
    QQmlProperty m_property;
    qreal m_to = 0;
    qreal m_value = 0;
    qreal m_velocity = 0;
    qreal m_stiffness = 300;
    qreal m_damping = 34;
    qreal m_mass = 1;
    qreal m_epsilon = 0.001;
    qreal m_accumulator = 0;
    Preset m_preset = Smooth;
    bool m_enabled = true;
    bool m_complete = false;
    bool m_toSet = false;
    bool m_valueSet = false;
    bool m_respectReducedMotion = true;
};

} // namespace Lumen
