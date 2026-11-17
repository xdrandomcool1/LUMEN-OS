// SPDX-License-Identifier: GPL-3.0-or-later
#include "layersurface.h"

#include <LayerShellQt/Window>
#include <QGuiApplication>
#include <QScreen>

namespace Lumen {

LayerSurface::LayerSurface(QObject *parent)
    : QObject(parent)
{
}

bool LayerSurface::isLayerShell()
{
    return QGuiApplication::platformName().startsWith(QLatin1String("wayland"));
}

void LayerSurface::setWindow(QQuickWindow *w)
{
    if (m_window == w)
        return;
    m_window = w;
    if (m_window) {
        m_window->setColor(Qt::transparent);
        if (!isLayerShell()) {
            m_window->setFlags(m_window->flags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
                               | (m_layer == Background ? Qt::WindowStaysOnBottomHint : Qt::Widget)
                               | Qt::WindowDoesNotAcceptFocus);
            connect(m_window, &QWindow::widthChanged, this, &LayerSurface::applyFallbackGeometry);
            connect(m_window, &QWindow::heightChanged, this, &LayerSurface::applyFallbackGeometry);
        }
    }
    Q_EMIT windowChanged();
    apply();
}

void LayerSurface::setLayer(Layer l) { if (m_layer != l) { m_layer = l; Q_EMIT changed(); apply(); } }
void LayerSurface::setAnchors(int a) { if (m_anchors != a) { m_anchors = a; Q_EMIT changed(); apply(); } }
void LayerSurface::setExclusiveZone(int z) { if (m_exclusiveZone != z) { m_exclusiveZone = z; Q_EMIT changed(); apply(); } }
void LayerSurface::setKeyboard(Keyboard k) { if (m_keyboard != k) { m_keyboard = k; Q_EMIT changed(); apply(); } }
void LayerSurface::setScope(const QString &s) { if (m_scope != s) { m_scope = s; Q_EMIT changed(); apply(); } }
QScreen *LayerSurface::resolveScreen(QObject *o)
{
    if (!o)
        return nullptr;
    if (auto *s = qobject_cast<QScreen *>(o))
        return s;
    const QString name = o->property("name").toString();
    for (QScreen *s : QGuiApplication::screens())
        if (s->name() == name)
            return s;
    return nullptr;
}

void LayerSurface::setScreen(QObject *o)
{
    QScreen *s = resolveScreen(o);
    if (m_screen == s)
        return;
    m_screen = s;
    Q_EMIT changed();
    apply();
}
void LayerSurface::setMargin(int i, int v) { if (m_margins[i] != v) { m_margins[i] = v; Q_EMIT changed(); apply(); } }

void LayerSurface::setInputRegion(const QRectF &r)
{
    if (m_inputRegion == r)
        return;
    m_inputRegion = r;
    Q_EMIT changed();
    if (m_window)
        m_window->setMask(r.isEmpty() ? QRegion() : QRegion(r.toAlignedRect()));
}

void LayerSurface::apply()
{
    if (!m_window)
        return;
    if (m_screen)
        m_window->setScreen(m_screen);
    if (!m_inputRegion.isEmpty())
        m_window->setMask(QRegion(m_inputRegion.toAlignedRect()));
    if (!isLayerShell()) {
        applyFallbackGeometry();
        return;
    }
    auto *lw = LayerShellQt::Window::get(m_window);
    if (!lw)
        return;
    lw->setScope(m_scope);
    lw->setLayer(LayerShellQt::Window::Layer(m_layer));
    LayerShellQt::Window::Anchors a;
    if (m_anchors & AnchorTop) a |= LayerShellQt::Window::AnchorTop;
    if (m_anchors & AnchorBottom) a |= LayerShellQt::Window::AnchorBottom;
    if (m_anchors & AnchorLeft) a |= LayerShellQt::Window::AnchorLeft;
    if (m_anchors & AnchorRight) a |= LayerShellQt::Window::AnchorRight;
    lw->setAnchors(a);
    lw->setExclusiveZone(m_exclusiveZone);
    lw->setMargins(QMargins(m_margins[2], m_margins[0], m_margins[3], m_margins[1]));
    lw->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivity(m_keyboard));
    lw->setWantsToBeOnActiveScreen(!m_screen);
    if (m_screen)
        lw->setScreen(m_screen);
}

void LayerSurface::applyFallbackGeometry()
{
    if (!m_window || isLayerShell())
        return;
    QScreen *s = m_screen ? m_screen.data() : QGuiApplication::primaryScreen();
    if (!s)
        return;
    const QRect g = s->geometry();
    QRect r(QPoint(0, 0), m_window->size());
    const bool l = m_anchors & AnchorLeft, rt = m_anchors & AnchorRight;
    const bool t = m_anchors & AnchorTop, b = m_anchors & AnchorBottom;
    if (l && rt) { r.setLeft(g.left() + m_margins[2]); r.setRight(g.right() - m_margins[3]); }
    else if (l) r.moveLeft(g.left() + m_margins[2]);
    else if (rt) r.moveRight(g.right() - m_margins[3]);
    else r.moveLeft(g.left() + (g.width() - r.width()) / 2);
    if (t && b) { r.setTop(g.top() + m_margins[0]); r.setBottom(g.bottom() - m_margins[1]); }
    else if (t) r.moveTop(g.top() + m_margins[0]);
    else if (b) r.moveBottom(g.bottom() - m_margins[1]);
    else r.moveTop(g.top() + (g.height() - r.height()) / 2);
    if (m_window->geometry() != r)
        m_window->setGeometry(r);
}

} // namespace Lumen
