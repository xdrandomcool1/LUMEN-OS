// SPDX-License-Identifier: GPL-3.0-or-later
// LUMEN Liquid Glass for KWin — see glass.h. Capture and repaint handling
// follow KWin's own blur effect (src/plugins/blur), so the glass behaves
// exactly like blur-behind with respect to damage, caching and opacity.
#include "glass.h"

#include <core/output.h>
#include <core/rendertarget.h>
#include <core/renderviewport.h>
#include <effect/effecthandler.h>
#include <effect/effectwindow.h>
#include <layershellv1window.h>
#include <opengl/eglcontext.h>
#include <scene/backgroundeffectitem.h>
#include <wayland/layershell_v1.h>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>

#include <cmath>

Q_LOGGING_CATEGORY(LUMEN_GLASS, "lumen.kwin.glass", QtWarningMsg)

static void ensureResources()
{
    Q_INIT_RESOURCE(glass);
}

using namespace KWin;

namespace Lumen {

static const QString s_path = QStringLiteral("/org/lumen/Glass");
static const QString s_interface = QStringLiteral("org.lumen.Glass");
static constexpr int s_maxLevels = 6;

GlassEffect::GlassEffect()
{
    ensureResources();
    m_shader = ShaderManager::instance()->generateShaderFromFile(ShaderTrait::MapTexture,
                                                                 QStringLiteral(":/effects/lumen_glass/shaders/glass.vert"),
                                                                 QStringLiteral(":/effects/lumen_glass/shaders/glass.frag"));
    if (!m_shader) {
        qCWarning(LUMEN_GLASS) << "Failed to load the glass shader";
        return;
    }
    m_loc.mvp = m_shader->uniformLocation("modelViewProjectionMatrix");
    m_loc.texSize = m_shader->uniformLocation("texSize");
    m_loc.shape = m_shader->uniformLocation("shape");
    m_loc.radius = m_shader->uniformLocation("radius");
    m_loc.smoothness = m_shader->uniformLocation("smoothness");
    m_loc.thickness = m_shader->uniformLocation("thickness");
    m_loc.lensWidth = m_shader->uniformLocation("lensWidth");
    m_loc.dispersion = m_shader->uniformLocation("dispersion");
    m_loc.ior = m_shader->uniformLocation("ior");
    m_loc.frost = m_shader->uniformLocation("frost");
    m_loc.saturation = m_shader->uniformLocation("saturation");
    m_loc.brightness = m_shader->uniformLocation("brightness");
    m_loc.opacity = m_shader->uniformLocation("opacity");
    m_valid = true;

    connect(effects, &EffectsHandler::windowAdded, this, &GlassEffect::windowAdded);
    connect(effects, &EffectsHandler::windowDeleted, this, &GlassEffect::windowDeleted);
    const auto windows = effects->stackingOrder();
    for (EffectWindow *w : windows)
        windowAdded(w);

    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerObject(s_path, this, QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals))
        qCWarning(LUMEN_GLASS) << "Could not register" << s_path << bus.lastError().message();
    // Tell running shells to (re-)publish their shapes.
    Q_EMIT Started();
}

GlassEffect::~GlassEffect()
{
    QDBusConnection::sessionBus().unregisterObject(s_path);
    effects->makeOpenGLContextCurrent();
    m_windows.clear();
}

bool GlassEffect::supported()
{
    return effects->isOpenGLCompositing();
}

bool GlassEffect::isActive() const
{
    return m_valid && !m_windows.empty();
}

QString GlassEffect::keyFor(EffectWindow *w)
{
    auto *layer = qobject_cast<LayerShellV1Window *>(w->window());
    if (!layer || !layer->shellSurface())
        return {};
    const QString scope = layer->shellSurface()->scope();
    if (!scope.startsWith(QLatin1String("lumen-")))
        return {};
    const QString output = w->screen() ? w->screen()->name() : QString();
    return scope + QLatin1Char('|') + output;
}

void GlassEffect::windowAdded(EffectWindow *w)
{
    // The output of a layer surface is fixed at creation, but be safe.
    connect(w, &EffectWindow::windowFrameGeometryChanged, this, [this, w] { applyShapes(w); });
    applyShapes(w);
}

void GlassEffect::windowDeleted(EffectWindow *w)
{
    if (auto it = m_windows.find(w); it != m_windows.end()) {
        effects->makeOpenGLContextCurrent();
        m_windows.erase(it);
    }
}

void GlassEffect::applyShapes(EffectWindow *w)
{
    const QString key = keyFor(w);
    const QList<GlassShape> shapes = key.isEmpty() ? QList<GlassShape>() : m_published.value(key);
    if (shapes.isEmpty()) {
        if (auto it = m_windows.find(w); it != m_windows.end()) {
            effects->makeOpenGLContextCurrent();
            m_windows.erase(it);
            w->addRepaintFull();
        }
        return;
    }
    WindowData &data = m_windows[w];
    data.shapes = shapes;
    if (!data.item)
        data.item = std::make_unique<BackgroundEffectItem>(w->windowItem());
    RectF bounds;
    for (const GlassShape &s : shapes)
        bounds = bounds.united(RectF(s.x, s.y, s.w, s.h));
    // Any damage under the glass repaints all of it: refraction samples
    // across the whole shape, so a partial update would read stale pixels.
    data.item->setEffectBoundingRect(bounds);
    w->addRepaintFull();
}

void GlassEffect::SetShapes(const QString &scope, const QString &output, const QString &json)
{
    QList<GlassShape> shapes;
    const QJsonArray arr = QJsonDocument::fromJson(json.toUtf8()).array();
    for (const QJsonValue &v : arr) {
        const QJsonObject o = v.toObject();
        GlassShape s;
        s.x = o.value(QLatin1String("x")).toDouble();
        s.y = o.value(QLatin1String("y")).toDouble();
        s.w = o.value(QLatin1String("w")).toDouble();
        s.h = o.value(QLatin1String("h")).toDouble();
        if (s.w <= 1 || s.h <= 1)
            continue;
        s.radius = o.value(QLatin1String("r")).toDouble(16);
        s.smoothness = o.value(QLatin1String("n")).toDouble(2.6);
        s.thickness = o.value(QLatin1String("thick")).toDouble(20);
        s.lensWidth = o.value(QLatin1String("lens")).toDouble(20);
        s.dispersion = o.value(QLatin1String("disp")).toDouble(0.5);
        s.ior = o.value(QLatin1String("ior")).toDouble(1.5);
        s.frost = std::clamp(o.value(QLatin1String("frost")).toDouble(0), 0.0, 1.0);
        s.saturation = o.value(QLatin1String("sat")).toDouble(1.2);
        s.brightness = o.value(QLatin1String("bri")).toDouble(1.0);
        shapes.append(s);
    }
    const QString key = scope + QLatin1Char('|') + output;
    if (shapes.isEmpty())
        m_published.remove(key);
    else
        m_published.insert(key, shapes);

    const auto windows = effects->stackingOrder();
    for (EffectWindow *w : windows) {
        if (keyFor(w) == key)
            applyShapes(w);
    }
}

void GlassEffect::prePaintScreen(ScreenPrePaintData &data)
{
    m_currentView = data.view;
    effects->prePaintScreen(data);
}

bool GlassEffect::drawWindow(const RenderTarget &renderTarget, const RenderViewport &viewport, EffectWindow *w,
                             int mask, const Region &deviceRegion, WindowPaintData &data)
{
    render(renderTarget, viewport, w, mask, deviceRegion, data);
    // The shell window (tint, light, content) is drawn over the refracted backdrop.
    return effects->drawWindow(renderTarget, viewport, w, mask, deviceRegion, data);
}

void GlassEffect::render(const RenderTarget &renderTarget, const RenderViewport &viewport, EffectWindow *w,
                         int mask, const Region &deviceRegion, WindowPaintData &data)
{
    auto it = m_windows.find(w);
    if (!m_valid || it == m_windows.end())
        return;
    // Transformed windows (overview, minimise animations) would need the
    // shapes transformed as well; like blur, skip them.
    const bool scaled = !qFuzzyCompare(data.xScale(), 1.0) || !qFuzzyCompare(data.yScale(), 1.0);
    const bool translated = data.xTranslation() || data.yTranslation();
    if (scaled || translated || (mask & PAINT_WINDOW_TRANSFORMED))
        return;

    WindowData &info = it->second;
    RenderData &rd = info.render[m_currentView];

    RectF shapeBounds;
    for (const GlassShape &s : info.shapes)
        shapeBounds = shapeBounds.united(RectF(s.x, s.y, s.w, s.h));
    const RectF globalBounds = shapeBounds.translated(w->pos());
    const Rect backgroundRect = globalBounds.rounded();
    if (backgroundRect.isEmpty())
        return;
    const double scale = viewport.scale();
    const Rect scaledBackgroundRect = backgroundRect.scaled(scale).rounded();
    const Rect deviceBackgroundRect = viewport.mapToDeviceCoordinates(RectF(backgroundRect)).rounded();
    const QSize texSize = scaledBackgroundRect.size();
    if (texSize.isEmpty())
        return;

    // Clip to what is actually being repainted.
    QList<RectF> effectiveShape;
    const RectF localDevice(0, 0, texSize.width(), texSize.height());
    if (deviceRegion != Region::infinite()) {
        for (const Rect &clip : deviceRegion.rects()) {
            const RectF r = RectF(clip.translated(-deviceBackgroundRect.topLeft())).intersected(localDevice);
            if (!r.isEmpty())
                effectiveShape.append(r);
        }
    } else {
        effectiveShape.append(localDevice);
    }
    if (effectiveShape.isEmpty())
        return;

    bool frosted = false;
    for (const GlassShape &s : info.shapes)
        frosted = frosted || s.frost > 0.01f;
    const int levels = frosted ? s_maxLevels : 1;

    GLenum format = GL_RGBA8;
    if (renderTarget.texture())
        format = renderTarget.texture()->internalFormat();
    if (!rd.texture || rd.texture->size() != texSize || rd.texture->internalFormat() != format || rd.levels != levels) {
        rd.framebuffer.reset();
        rd.texture = GLTexture::allocate(format, texSize, levels);
        if (!rd.texture) {
            qCWarning(LUMEN_GLASS) << "Failed to allocate the backdrop texture";
            return;
        }
        rd.levels = levels;
        rd.texture->setFilter(levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        rd.texture->setWrapMode(GL_CLAMP_TO_EDGE);
        rd.framebuffer = std::make_unique<GLFramebuffer>(rd.texture.get());
        if (!rd.framebuffer->valid()) {
            qCWarning(LUMEN_GLASS) << "Failed to create the backdrop framebuffer";
            rd.framebuffer.reset();
            rd.texture.reset();
            return;
        }
        EglContext::currentContext()->pushFramebuffer(rd.framebuffer.get());
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        EglContext::currentContext()->popFramebuffer();
    }

    // Copy what has already been composited behind the glass (all windows
    // below this one). Undamaged parts are still valid from the last frame.
    const Region dirty = viewport.mapFromDeviceCoordinatesContained(deviceRegion) & backgroundRect;
    for (const Rect &r : dirty.rects()) {
        const Rect dst = r.translated(-backgroundRect.topLeft()).scaled(scale).rounded();
        rd.framebuffer->blitFromRenderTarget(renderTarget, viewport, r, dst);
    }
    if (levels > 1)
        rd.texture->generateMipmaps();

    GLVertexBuffer *vbo = GLVertexBuffer::streamingBuffer();
    vbo->reset();
    vbo->setAttribLayout(std::span(GLVertexBuffer::GLVertex2DLayout), sizeof(GLVertex2D));
    const int vertexCount = effectiveShape.size() * 6;
    if (auto result = vbo->map<GLVertex2D>(vertexCount)) {
        auto map = *result;
        size_t i = 0;
        for (const RectF &r : effectiveShape) {
            const float x0 = r.left(), y0 = r.top(), x1 = r.right(), y1 = r.bottom();
            const auto v = [&](float x, float y) {
                // texcoord carries device px relative to the backdrop (y down).
                map[i++] = GLVertex2D{.position = QVector2D(x, y), .texcoord = QVector2D(x, y)};
            };
            v(x0, y0); v(x1, y1); v(x0, y1);
            v(x0, y0); v(x1, y0); v(x1, y1);
        }
        vbo->unmap();
    } else {
        qCWarning(LUMEN_GLASS) << "Failed to map the vertex buffer";
        return;
    }

    vbo->bindArrays();
    ShaderManager::instance()->pushShader(m_shader.get());
    QMatrix4x4 projection = viewport.projectionMatrix();
    projection.translate(scaledBackgroundRect.x(), scaledBackgroundRect.y());
    m_shader->setUniform(m_loc.mvp, projection);
    m_shader->setUniform(m_loc.texSize, QVector2D(texSize.width(), texSize.height()));
    const float opacity = float(w->opacity() * data.opacity());
    m_shader->setUniform(m_loc.opacity, opacity);
    rd.texture->bind();

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    for (const GlassShape &s : info.shapes) {
        const QVector4D shape(float((s.x - shapeBounds.x()) * scale), float((s.y - shapeBounds.y()) * scale),
                              float(s.w * scale), float(s.h * scale));
        m_shader->setUniform(m_loc.shape, shape);
        m_shader->setUniform(m_loc.radius, float(s.radius * scale));
        m_shader->setUniform(m_loc.smoothness, s.smoothness);
        m_shader->setUniform(m_loc.thickness, float(s.thickness * scale));
        m_shader->setUniform(m_loc.lensWidth, float(s.lensWidth * scale));
        m_shader->setUniform(m_loc.dispersion, s.dispersion);
        m_shader->setUniform(m_loc.ior, s.ior);
        m_shader->setUniform(m_loc.frost, levels > 1 ? s.frost : 0.0f);
        m_shader->setUniform(m_loc.saturation, s.saturation);
        m_shader->setUniform(m_loc.brightness, s.brightness);
        vbo->draw(GL_TRIANGLES, 0, vertexCount);
    }
    glDisable(GL_BLEND);
    ShaderManager::instance()->popShader();
    vbo->unbindArrays();
}

} // namespace Lumen

KWIN_EFFECT_FACTORY_SUPPORTED_ENABLED(Lumen::GlassEffect, "metadata.json", return Lumen::GlassEffect::supported();, return Lumen::GlassEffect::enabledByDefault();)

#include "glass.moc"
