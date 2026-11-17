// SPDX-License-Identifier: GPL-3.0-or-later
// Lumen.Glass — publishes glass shapes of a shell surface to the
// compositor-side Liquid Glass effect (KWin effect "lumen_glass", see
// compositor/effects/glass/glass.h), which refracts the real content behind
// the surface — other windows included.
//
//   CompositorGlass {
//       scope: "lumen-dock"                     // the LayerSurface scope
//       output: targetScreen.name
//       shapes: [ dockGlass.compositorShape(dockRect) ]
//   }
//
// `active` is true while the effect is loaded and accepting shapes; the
// surface then draws only tint and light (GlassSurface.compositorBlur with
// the window property lumenCompositorGlass). Otherwise callers fall back to
// in-process refraction of the wallpaper or to compositor blur.
#pragma once

#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

namespace Lumen {

class CompositorGlass : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString scope READ scope WRITE setScope NOTIFY changed)
    Q_PROPERTY(QString output READ output WRITE setOutput NOTIFY changed)
    Q_PROPERTY(QVariantList shapes READ shapes WRITE setShapes NOTIFY changed)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY changed)
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(bool active READ active NOTIFY availableChanged)

public:
    explicit CompositorGlass(QObject *parent = nullptr);
    ~CompositorGlass() override;

    QString scope() const { return m_scope; }
    void setScope(const QString &s);
    QString output() const { return m_output; }
    void setOutput(const QString &o);
    QVariantList shapes() const { return m_shapes; }
    void setShapes(const QVariantList &s);
    bool enabled() const { return m_enabled; }
    void setEnabled(bool e);
    //! The KWin effect is loaded (independent of `enabled`).
    bool available() const;
    bool active() const { return m_enabled && available(); }

Q_SIGNALS:
    void changed();
    void availableChanged();

private:
    void schedule();
    void send();
    void sendEmpty(const QString &scope, const QString &output);

    QString m_scope;
    QString m_output;
    QVariantList m_shapes;
    bool m_enabled = true;
    QString m_sentScope, m_sentOutput; // last key that holds shapes in KWin
    QTimer m_timer;
};

} // namespace Lumen
