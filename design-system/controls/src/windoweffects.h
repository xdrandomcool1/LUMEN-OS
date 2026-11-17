// SPDX-License-Identifier: GPL-3.0-or-later
// Lumen.Controls — compositor-side window effects (shadow, blur behind).
#pragma once

#include <QObject>
#include <QPointer>
#include <QQuickWindow>
#include <QRectF>
#include <QtQml/qqmlregistration.h>
#include <memory>

class KWindowShadow;

namespace Lumen {

/*!
 * Attaches compositor effects to a QWindow:
 *  - a soft, rounded drop shadow drawn by KWin outside the window
 *    (org_kde_kwin_shadow), so frameless Lumen windows keep correct input
 *    geometry and snapping while still casting a macOS-like shadow;
 *  - blur behind a rounded region (org_kde_kwin_blur) for glass surfaces
 *    whose backdrop belongs to another client.
 */
class WindowEffects : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QQuickWindow *window READ window WRITE setWindow NOTIFY windowChanged)
    Q_PROPERTY(bool shadow READ shadow WRITE setShadow NOTIFY changed)
    Q_PROPERTY(qreal radius READ radius WRITE setRadius NOTIFY changed)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY changed)
    Q_PROPERTY(bool blurBehind READ blurBehind WRITE setBlurBehind NOTIFY changed)
    Q_PROPERTY(QRectF blurRect READ blurRect WRITE setBlurRect NOTIFY changed)
    Q_PROPERTY(qreal blurRadius READ blurRadius WRITE setBlurRadius NOTIFY changed)

public:
    explicit WindowEffects(QObject *parent = nullptr);
    ~WindowEffects() override;

    QQuickWindow *window() const { return m_window; }
    void setWindow(QQuickWindow *w);
    bool shadow() const { return m_shadow; }
    void setShadow(bool s);
    qreal radius() const { return m_radius; }
    void setRadius(qreal r);
    bool active() const { return m_active; }
    void setActive(bool a);
    bool blurBehind() const { return m_blur; }
    void setBlurBehind(bool b);
    QRectF blurRect() const { return m_blurRect; }
    void setBlurRect(const QRectF &r);
    qreal blurRadius() const { return m_blurRadius; }
    void setBlurRadius(qreal r);

    //! Rounded-rect region approximated with horizontal spans (for blur).
    static QRegion roundedRegion(const QRect &rect, int radius);

Q_SIGNALS:
    void windowChanged();
    void changed();

private:
    void apply();
    void applyShadow();
    void applyBlur();

    QPointer<QQuickWindow> m_window;
    std::unique_ptr<KWindowShadow> m_kshadow;
    bool m_shadow = true;
    bool m_active = true;
    bool m_blur = false;
    qreal m_radius = 16;
    qreal m_blurRadius = -1;
    QRectF m_blurRect;
};

} // namespace Lumen
