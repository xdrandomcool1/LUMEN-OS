// SPDX-License-Identifier: GPL-3.0-or-later
#include "windoweffects.h"

#include <KWindowEffects>
#include <KWindowShadow>
#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

namespace Lumen {

namespace {

// Separable box blur ×3 ≈ Gaussian, on an alpha-only image.
QImage blurAlpha(const QImage &src, int radius)
{
    if (radius < 1)
        return src;
    QImage img = src.convertToFormat(QImage::Format_Alpha8);
    const int w = img.width(), h = img.height();
    QList<uchar> tmp(qMax(w, h));
    for (int pass = 0; pass < 3; ++pass) {
        for (int y = 0; y < h; ++y) {
            uchar *row = img.scanLine(y);
            int sum = 0;
            for (int i = -radius; i <= radius; ++i)
                sum += row[qBound(0, i, w - 1)];
            for (int x = 0; x < w; ++x) {
                tmp[x] = uchar(sum / (2 * radius + 1));
                sum += row[qMin(x + radius + 1, w - 1)] - row[qMax(x - radius, 0)];
            }
            memcpy(row, tmp.constData(), w);
        }
        for (int x = 0; x < w; ++x) {
            int sum = 0;
            for (int i = -radius; i <= radius; ++i)
                sum += img.scanLine(qBound(0, i, h - 1))[x];
            for (int y = 0; y < h; ++y) {
                tmp[y] = uchar(sum / (2 * radius + 1));
                sum += img.scanLine(qMin(y + radius + 1, h - 1))[x] - img.scanLine(qMax(y - radius, 0))[x];
            }
            for (int y = 0; y < h; ++y)
                img.scanLine(y)[x] = tmp[y];
        }
    }
    return img;
}

struct ShadowSpec { int size; int offsetY; qreal alpha; };

// Two-layer shadow (ambient + key), cut out under the window, as a 9-patch.
QImage makeShadowImage(qreal radius, bool active, int &padding, int &inner)
{
    const ShadowSpec ambient = active ? ShadowSpec{40, 18, 0.32} : ShadowSpec{28, 10, 0.18};
    const ShadowSpec key = active ? ShadowSpec{8, 3, 0.22} : ShadowSpec{6, 2, 0.14};
    padding = ambient.size + ambient.offsetY;
    inner = int(qCeil(radius)) + 2;
    const int core = inner * 2 + 1;
    const int side = padding * 2 + core;
    QImage canvas(side, side, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::transparent);

    auto layer = [&](const ShadowSpec &s) {
        QImage mask(side, side, QImage::Format_Alpha8);
        mask.fill(0);
        QPainter p(&mask);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, int(255 * s.alpha)));
        p.drawRoundedRect(QRectF(padding, padding + s.offsetY, core, core), radius, radius);
        p.end();
        const QImage blurred = blurAlpha(mask, s.size / 3);
        QPainter cp(&canvas);
        cp.drawImage(0, 0, blurred.convertToFormat(QImage::Format_ARGB32_Premultiplied));
    };
    layer(ambient);
    layer(key);

    // A 1 px outline ring gives windows a crisp edge on any background.
    {
        QPainter p(&canvas);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(QColor(0, 0, 0, active ? 70 : 45), 1.0));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(padding - 0.5, padding - 0.5, core + 1, core + 1), radius + 0.5, radius + 0.5);
        // Cut out the area under the window itself.
        p.setCompositionMode(QPainter::CompositionMode_Clear);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::black);
        p.drawRoundedRect(QRectF(padding, padding, core, core), radius, radius);
    }
    return canvas;
}

} // namespace

WindowEffects::WindowEffects(QObject *parent)
    : QObject(parent)
{
}

WindowEffects::~WindowEffects() = default;

void WindowEffects::setWindow(QQuickWindow *w)
{
    if (m_window == w)
        return;
    m_window = w;
    m_kshadow.reset();
    Q_EMIT windowChanged();
    if (m_window) {
        connect(m_window, &QWindow::visibleChanged, this, &WindowEffects::apply);
        connect(m_window, &QWindow::widthChanged, this, &WindowEffects::applyBlur);
        connect(m_window, &QWindow::heightChanged, this, &WindowEffects::applyBlur);
    }
    apply();
}

void WindowEffects::setShadow(bool s) { if (m_shadow != s) { m_shadow = s; Q_EMIT changed(); applyShadow(); } }
void WindowEffects::setRadius(qreal r) { if (!qFuzzyCompare(m_radius, r)) { m_radius = r; Q_EMIT changed(); apply(); } }
void WindowEffects::setActive(bool a) { if (m_active != a) { m_active = a; Q_EMIT changed(); applyShadow(); } }
void WindowEffects::setBlurBehind(bool b) { if (m_blur != b) { m_blur = b; Q_EMIT changed(); applyBlur(); } }
void WindowEffects::setBlurRect(const QRectF &r) { if (m_blurRect != r) { m_blurRect = r; Q_EMIT changed(); applyBlur(); } }
void WindowEffects::setBlurRadius(qreal r) { if (!qFuzzyCompare(m_blurRadius, r)) { m_blurRadius = r; Q_EMIT changed(); applyBlur(); } }

void WindowEffects::apply()
{
    applyShadow();
    applyBlur();
}

void WindowEffects::applyShadow()
{
    if (!m_window || !m_window->isVisible())
        return;
    if (!m_shadow) {
        m_kshadow.reset();
        return;
    }
    if (!QGuiApplication::platformName().startsWith(QLatin1String("wayland"))
        && !QGuiApplication::platformName().startsWith(QLatin1String("xcb")))
        return;

    int padding = 0, inner = 0;
    const QImage img = makeShadowImage(m_radius, m_active, padding, inner);
    const int side = img.width();
    const int core = side - 2 * padding;

    auto tile = [&](const QRect &r) {
        auto t = KWindowShadowTile::Ptr::create();
        t->setImage(img.copy(r));
        return t;
    };
    const int a = padding + inner; // corner tile extent
    const int mid = side - 2 * a;

    m_kshadow = std::make_unique<KWindowShadow>();
    m_kshadow->setTopLeftTile(tile({0, 0, a, a}));
    m_kshadow->setTopTile(tile({a, 0, mid, padding}));
    m_kshadow->setTopRightTile(tile({side - a, 0, a, a}));
    m_kshadow->setRightTile(tile({side - padding, a, padding, mid}));
    m_kshadow->setBottomRightTile(tile({side - a, side - a, a, a}));
    m_kshadow->setBottomTile(tile({a, side - padding, mid, padding}));
    m_kshadow->setBottomLeftTile(tile({0, side - a, a, a}));
    m_kshadow->setLeftTile(tile({0, a, padding, mid}));
    m_kshadow->setPadding(QMargins(padding - inner, padding - inner, padding - inner, padding - inner));
    Q_UNUSED(core)
    m_kshadow->setWindow(m_window);
    m_kshadow->create();
}

QRegion WindowEffects::roundedRegion(const QRect &rect, int radius)
{
    QRegion region;
    radius = qMin(radius, qMin(rect.width(), rect.height()) / 2);
    if (radius <= 0)
        return QRegion(rect);
    region += rect.adjusted(0, radius, 0, -radius);
    for (int i = 0; i < radius; ++i) {
        const qreal dy = radius - i - 0.5;
        const int dx = radius - int(qSqrt(qMax<qreal>(0, radius * radius - dy * dy)));
        region += QRect(rect.x() + dx, rect.y() + i, rect.width() - 2 * dx, 1);
        region += QRect(rect.x() + dx, rect.bottom() - i, rect.width() - 2 * dx, 1);
    }
    return region;
}

void WindowEffects::applyBlur()
{
    if (!m_window)
        return;
    if (!m_blur) {
        KWindowEffects::enableBlurBehind(m_window, false);
        return;
    }
    QRect r = m_blurRect.isValid() ? m_blurRect.toAlignedRect() : QRect(QPoint(0, 0), m_window->size());
    const int radius = int(m_blurRadius >= 0 ? m_blurRadius : m_radius);
    KWindowEffects::enableBlurBehind(m_window, true, roundedRegion(r, radius));
}

} // namespace Lumen
