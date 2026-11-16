// SPDX-License-Identifier: GPL-3.0-or-later
// lumen-shell — declarative wlr-layer-shell configuration for QML windows.
#pragma once

#include <QObject>
#include <QPointer>
#include <QQuickWindow>
#include <QtQml/qqmlregistration.h>

namespace Lumen {

/*!
 * Turns a QML Window into a layer-shell surface (wallpaper, menu bar, Dock,
 * panels). Must be bound before the window is first shown:
 *
 *   Window { id: w; visible: false
 *       LayerSurface { window: w; layer: LayerSurface.Top; anchors: LayerSurface.AnchorTop | ...
 *                      onReady: w.visible = true } }
 *
 * Off Wayland (X11 development sessions) it degrades to a frameless,
 * always-on-top window placed according to the anchors.
 */
class LayerSurface : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QQuickWindow *window READ window WRITE setWindow NOTIFY windowChanged)
    Q_PROPERTY(Layer layer READ layer WRITE setLayer NOTIFY changed)
    Q_PROPERTY(int anchors READ anchors WRITE setAnchors NOTIFY changed)
    Q_PROPERTY(int exclusiveZone READ exclusiveZone WRITE setExclusiveZone NOTIFY changed)
    Q_PROPERTY(Keyboard keyboard READ keyboard WRITE setKeyboard NOTIFY changed)
    Q_PROPERTY(QString scope READ scope WRITE setScope NOTIFY changed)
    Q_PROPERTY(int marginTop READ marginTop WRITE setMarginTop NOTIFY changed)
    Q_PROPERTY(int marginBottom READ marginBottom WRITE setMarginBottom NOTIFY changed)
    Q_PROPERTY(int marginLeft READ marginLeft WRITE setMarginLeft NOTIFY changed)
    Q_PROPERTY(int marginRight READ marginRight WRITE setMarginRight NOTIFY changed)
    //! Accepts a QScreen or a QML screen-info object (Qt.application.screens[i]).
    Q_PROPERTY(QObject *screen READ screenObject WRITE setScreen NOTIFY changed)
    Q_PROPERTY(bool isLayerShell READ isLayerShell CONSTANT)
    //! Input region in window coordinates; empty = whole window.
    Q_PROPERTY(QRectF inputRegion READ inputRegion WRITE setInputRegion NOTIFY changed)

public:
    enum Layer { Background, Bottom, Top, Overlay };
    Q_ENUM(Layer)
    enum Anchor { AnchorNone = 0, AnchorTop = 1, AnchorBottom = 2, AnchorLeft = 4, AnchorRight = 8 };
    Q_ENUM(Anchor)
    enum Keyboard { KeyboardNone, KeyboardExclusive, KeyboardOnDemand };
    Q_ENUM(Keyboard)

    explicit LayerSurface(QObject *parent = nullptr);

    QQuickWindow *window() const { return m_window; }
    void setWindow(QQuickWindow *w);
    Layer layer() const { return m_layer; }
    void setLayer(Layer l);
    int anchors() const { return m_anchors; }
    void setAnchors(int a);
    int exclusiveZone() const { return m_exclusiveZone; }
    void setExclusiveZone(int z);
    Keyboard keyboard() const { return m_keyboard; }
    void setKeyboard(Keyboard k);
    QString scope() const { return m_scope; }
    void setScope(const QString &s);
    int marginTop() const { return m_margins[0]; }
    int marginBottom() const { return m_margins[1]; }
    int marginLeft() const { return m_margins[2]; }
    int marginRight() const { return m_margins[3]; }
    void setMarginTop(int v) { setMargin(0, v); }
    void setMarginBottom(int v) { setMargin(1, v); }
    void setMarginLeft(int v) { setMargin(2, v); }
    void setMarginRight(int v) { setMargin(3, v); }
    QScreen *screen() const { return m_screen; }
    QObject *screenObject() const { return m_screen; }
    void setScreen(QObject *s);
    static QScreen *resolveScreen(QObject *o);
    static bool isLayerShell();
    QRectF inputRegion() const { return m_inputRegion; }
    void setInputRegion(const QRectF &r);

Q_SIGNALS:
    void windowChanged();
    void changed();

private:
    void setMargin(int i, int v);
    void apply();
    void applyFallbackGeometry();

    QPointer<QQuickWindow> m_window;
    QPointer<QScreen> m_screen;
    Layer m_layer = Top;
    int m_anchors = 0;
    int m_exclusiveZone = 0;
    Keyboard m_keyboard = KeyboardNone;
    QString m_scope = QStringLiteral("lumen");
    int m_margins[4] = {0, 0, 0, 0};
    QRectF m_inputRegion;
};

} // namespace Lumen
