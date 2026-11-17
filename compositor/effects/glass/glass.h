// SPDX-License-Identifier: GPL-3.0-or-later
// LUMEN Liquid Glass for KWin.
//
// Shell surfaces (layer-shell windows whose scope starts with "lumen-") whose
// glass sits over *other* clients — the Dock over a maximised window, Control
// Center over a browser — cannot refract in-process: the shell only sees its
// own wallpaper copy. This effect does it in the compositor: before such a
// window is drawn, the already composited pixels behind each glass shape are
// copied into a texture and drawn back refracted through a convex squircle
// bevel (Snell's law, per-channel IOR), optionally frosted via the texture's
// mip chain. The shell then paints only tint and light on top.
//
// The shell publishes its glass shapes over D-Bus on KWin's connection:
//   service org.kde.KWin, path /org/lumen/Glass, interface org.lumen.Glass
//   SetShapes(scope, output, json)   json: [{x,y,w,h,r,n,thick,lens,disp,ior,frost,sat,bri}]
//                                    (window-local logical px; [] clears)
//   Available() → true              (probe; the object exists only while
//                                    the effect is loaded)
//   signal Started()                 (emitted on load: shells re-send shapes)
#pragma once

#include <effect/effect.h>
#include <opengl/glutils.h>

#include <QHash>
#include <QList>

#include <memory>
#include <unordered_map>

namespace KWin {
class BackgroundEffectItem;
class RenderView;
}

namespace Lumen {

struct GlassShape
{
    double x = 0, y = 0, w = 0, h = 0; // window-local, logical px
    float radius = 16;
    float smoothness = 2.6f;
    float thickness = 20;
    float lensWidth = 20;
    float dispersion = 0.5f;
    float ior = 1.5f;
    float frost = 0;
    float saturation = 1.2f;
    float brightness = 1.0f;
};

class GlassEffect : public KWin::Effect
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.lumen.Glass")

public:
    GlassEffect();
    ~GlassEffect() override;

    static bool supported();
    static bool enabledByDefault() { return true; }

    void prePaintScreen(KWin::ScreenPrePaintData &data) override;
    bool drawWindow(const KWin::RenderTarget &renderTarget, const KWin::RenderViewport &viewport, KWin::EffectWindow *w,
                    int mask, const KWin::Region &deviceRegion, KWin::WindowPaintData &data) override;
    bool isActive() const override;
    int requestedEffectChainPosition() const override { return 20; } // same slot as blur

public Q_SLOTS:
    Q_SCRIPTABLE void SetShapes(const QString &scope, const QString &output, const QString &json);
    Q_SCRIPTABLE bool Available() const { return m_valid; }

Q_SIGNALS:
    Q_SCRIPTABLE void Started();

private:
    struct RenderData
    {
        std::unique_ptr<KWin::GLTexture> texture;
        std::unique_ptr<KWin::GLFramebuffer> framebuffer;
        int levels = 1;
    };
    struct WindowData
    {
        QList<GlassShape> shapes;
        std::unique_ptr<KWin::BackgroundEffectItem> item;
        std::unordered_map<KWin::RenderView *, RenderData> render;
    };

    static QString keyFor(KWin::EffectWindow *w);
    void windowAdded(KWin::EffectWindow *w);
    void windowDeleted(KWin::EffectWindow *w);
    void applyShapes(KWin::EffectWindow *w);
    void render(const KWin::RenderTarget &renderTarget, const KWin::RenderViewport &viewport, KWin::EffectWindow *w,
                int mask, const KWin::Region &deviceRegion, KWin::WindowPaintData &data);

    std::unique_ptr<KWin::GLShader> m_shader;
    struct
    {
        int mvp, texSize, shape, radius, smoothness, thickness, lensWidth, dispersion, ior, frost, saturation, brightness, opacity;
    } m_loc{};
    bool m_valid = false;
    KWin::RenderView *m_currentView = nullptr;
    QHash<QString, QList<GlassShape>> m_published; // key: scope|output
    std::unordered_map<KWin::EffectWindow *, WindowData> m_windows;
};

} // namespace Lumen
