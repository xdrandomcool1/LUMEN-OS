// SPDX-License-Identifier: GPL-3.0-or-later
// Lumen.Design — renderer detection shared by the shell, apps and SDDM.
#pragma once

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

class QQmlEngine;
class QJSEngine;

namespace Lumen {

/*!
 * Probes the OpenGL renderer once per process and classifies the machine
 * into a performance tier. Full is a real GPU; Lite is a paravirtualised
 * one (virtio/virgl/venus/SVGA); Software is llvmpipe & friends, where the
 * whole Liquid Glass pipeline would run on the CPU. The tier never changes
 * at runtime, so the properties are CONSTANT. LUMEN_PERF=full|lite|software
 * overrides the probe (auto by default).
 */
class Gpu : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(Tier tier READ tier CONSTANT)
    Q_PROPERTY(bool full READ full CONSTANT)
    Q_PROPERTY(bool lite READ lite CONSTANT)
    Q_PROPERTY(bool software READ software CONSTANT)
    Q_PROPERTY(QString renderer READ renderer CONSTANT)

public:
    enum Tier { Full, Lite, Software };
    Q_ENUM(Tier)

    static Gpu *instance();
    static Gpu *create(QQmlEngine *, QJSEngine *);

    Tier tier() const { return m_tier; }
    bool full() const { return m_tier == Full; }
    bool lite() const { return m_tier == Lite; }
    bool software() const { return m_tier == Software; }
    QString renderer() const { return m_renderer; }

    //! Classifies a GL_RENDERER string into a tier. Pure, unit-tested.
    static Tier classify(const QString &renderer);

private:
    explicit Gpu(QObject *parent = nullptr);

    Tier m_tier = Software;
    QString m_renderer;
};

} // namespace Lumen
