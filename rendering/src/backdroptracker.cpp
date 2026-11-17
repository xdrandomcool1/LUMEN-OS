// SPDX-License-Identifier: GPL-3.0-or-later
#include "backdroptracker.h"

#include <QQuickWindow>

namespace Lumen {

BackdropTracker::BackdropTracker(QObject *parent)
    : QObject(parent)
{
}

void BackdropTracker::setItem(QQuickItem *i)
{
    if (m_item == i)
        return;
    if (m_item)
        disconnect(m_item, nullptr, this, nullptr);
    m_item = i;
    if (m_item)
        connect(m_item, &QQuickItem::windowChanged, this, &BackdropTracker::connectWindow);
    connectWindow();
    Q_EMIT itemChanged();
    update();
}

void BackdropTracker::setReference(QQuickItem *r)
{
    if (m_reference == r)
        return;
    m_reference = r;
    Q_EMIT referenceChanged();
    update();
}

void BackdropTracker::connectWindow()
{
    QQuickWindow *w = m_item ? m_item->window() : nullptr;
    if (w == m_window)
        return;
    disconnect(m_frameConnection);
    m_window = w;
    if (m_window)
        m_frameConnection = connect(m_window, &QQuickWindow::afterAnimating, this,
                                    &BackdropTracker::update, Qt::DirectConnection);
}

void BackdropTracker::update()
{
    if (!m_item || !m_reference)
        return;
    const QRectF r = m_item->mapRectToItem(m_reference, QRectF(0, 0, m_item->width(), m_item->height()));
    const qreal rw = qMax<qreal>(1.0, m_reference->width());
    const qreal rh = qMax<qreal>(1.0, m_reference->height());
    const QRectF n(r.x() / rw, r.y() / rh, r.width() / rw, r.height() / rh);
    if (r == m_rect && n == m_normalized)
        return;
    m_rect = r;
    m_normalized = n;
    Q_EMIT rectChanged();
}

} // namespace Lumen
