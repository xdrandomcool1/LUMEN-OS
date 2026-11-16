// SPDX-License-Identifier: GPL-3.0-or-later
// Reports whether any visible window intersects a screen rectangle.
#pragma once
#include <QAbstractItemModel>
#include <QPointer>
#include <QRectF>
#include <QtQml/qqmlregistration.h>

namespace Lumen {

/*!
 * Glass in the shell can refract the wallpaper in-process (pixel-exact), but
 * when an application window slides under the Dock or a panel only the
 * compositor has those pixels. RegionWatcher observes a TasksModel and
 * flips `occupied` when a non-minimized window overlaps `region` so the
 * surface can switch to compositor blur.
 */
class RegionWatcher : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QAbstractItemModel *model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(QRectF region READ region WRITE setRegion NOTIFY regionChanged)
    Q_PROPERTY(bool occupied READ occupied NOTIFY occupiedChanged)
    Q_PROPERTY(bool anyMaximized READ anyMaximized NOTIFY occupiedChanged)
public:
    explicit RegionWatcher(QObject *parent = nullptr);
    QAbstractItemModel *model() const { return m_model; }
    void setModel(QAbstractItemModel *m);
    QRectF region() const { return m_region; }
    void setRegion(const QRectF &r);
    bool occupied() const { return m_occupied; }
    bool anyMaximized() const { return m_anyMax; }
Q_SIGNALS:
    void modelChanged();
    void regionChanged();
    void occupiedChanged();
private:
    void recompute();
    int role(const char *name) const;
    QPointer<QAbstractItemModel> m_model;
    QRectF m_region;
    bool m_occupied = false;
    bool m_anyMax = false;
};
}
