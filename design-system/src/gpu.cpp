// SPDX-License-Identifier: GPL-3.0-or-later
#include "gpu.h"

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QtGui/qopengl.h>
#include <QQmlEngine>

namespace Lumen {

namespace {

struct Detection {
    Gpu::Tier tier = Gpu::Software;
    QString renderer;
};

Detection probeRenderer()
{
    Detection d;
    QOpenGLContext context;
    if (!context.create())
        return d;
    QOffscreenSurface surface;
    surface.setFormat(context.format());
    surface.create();
    if (!surface.isValid())
        return d;
    if (!context.makeCurrent(&surface))
        return d;
    const char *raw = reinterpret_cast<const char *>(context.functions()->glGetString(GL_RENDERER));
    if (raw)
        d.renderer = QString::fromLatin1(raw);
    context.doneCurrent();
    if (d.renderer.isEmpty())
        return d;

    const QString s = d.renderer.toLower();
    if (s.contains(QLatin1String("llvmpipe")) || s.contains(QLatin1String("softpipe"))
        || s.contains(QLatin1String("swrast")) || s.contains(QLatin1String("lavapipe"))
        || s.contains(QLatin1String("swiftshader"))) {
        d.tier = Gpu::Software;
    } else if (s.contains(QLatin1String("virgl")) || s.contains(QLatin1String("venus"))
               || s.contains(QLatin1String("virtio")) || s.contains(QLatin1String("svga3d"))) {
        d.tier = Gpu::Lite;
    } else {
        d.tier = Gpu::Full;
    }
    return d;
}

} // namespace

Gpu::Gpu(QObject *parent)
    : QObject(parent)
{
    const Detection d = probeRenderer();

    const QString env = qEnvironmentVariable("LUMEN_PERF").toLower();
    if (env == QLatin1String("full") || env == QLatin1String("high"))
        m_tier = Full;
    else if (env == QLatin1String("lite"))
        m_tier = Lite;
    else if (env == QLatin1String("software") || env == QLatin1String("low"))
        m_tier = Software;
    else
        m_tier = d.tier;

    // The tier is the interesting bit; a nameless probe still means the
    // session runs without hardware acceleration.
    m_renderer = d.renderer.isEmpty() ? QStringLiteral("Software rendering") : d.renderer;
}

Gpu *Gpu::instance()
{
    static Gpu *s = new Gpu;
    return s;
}

Gpu *Gpu::create(QQmlEngine *, QJSEngine *)
{
    QQmlEngine::setObjectOwnership(instance(), QQmlEngine::CppOwnership);
    return instance();
}

} // namespace Lumen
