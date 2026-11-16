// SPDX-License-Identifier: GPL-3.0-or-later
// Lumen.Glass — tracks where a glass item sits on top of its backdrop.
#pragma once

#include <QPointer>
#include <QQuickItem>
#include <QRectF>
#include <QtQml/qqmlregistration.h>

namespace Lumen {

/*!
 * Computes `item`'s bounding rect in the coordinate system of `reference`
 * (the backdrop), normalized to the reference size. Recomputed once per frame
 * in afterAnimating, but notifies only on change, so a static UI costs nothing
 * and never forces extra frames.
 */
class BackdropTracker : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QQuickItem *item READ item WRITE setItem NOTIFY itemChanged)
    Q_PROPERTY(QQuickItem *reference READ reference WRITE setReference NOTIFY referenceChanged)
    Q_PROPERTY(QRectF rect READ rect NOTIFY rectChanged)            // in reference px
    Q_PROPERTY(QRectF normalizedRect READ normalizedRect NOTIFY rectChanged)

public:
    explicit BackdropTracker(QObject *parent = nullptr);

    QQuickItem *item() const { return m_item; }
    void setItem(QQuickItem *i);
    QQuickItem *reference() const { return m_reference; }
    void setReference(QQuickItem *r);
    QRectF rect() const { return m_rect; }
    QRectF normalizedRect() const { return m_normalized; }

    Q_INVOKABLE void update();

Q_SIGNALS:
    void itemChanged();
    void referenceChanged();
    void rectChanged();

private:
    void connectWindow();

    QPointer<QQuickItem> m_item;
    QPointer<QQuickItem> m_reference;
    QPointer<QQuickWindow> m_window;
    QMetaObject::Connection m_frameConnection;
    QRectF m_rect;
    QRectF m_normalized;
};

} // namespace Lumen
