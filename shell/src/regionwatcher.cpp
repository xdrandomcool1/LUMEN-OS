// SPDX-License-Identifier: GPL-3.0-or-later
#include "regionwatcher.h"

namespace Lumen {
RegionWatcher::RegionWatcher(QObject *parent) : QObject(parent) {}

void RegionWatcher::setModel(QAbstractItemModel *m)
{
    if (m_model == m) return;
    if (m_model) disconnect(m_model, nullptr, this, nullptr);
    m_model = m;
    if (m_model) {
        connect(m_model, &QAbstractItemModel::dataChanged, this, &RegionWatcher::recompute);
        connect(m_model, &QAbstractItemModel::rowsInserted, this, &RegionWatcher::recompute);
        connect(m_model, &QAbstractItemModel::rowsRemoved, this, &RegionWatcher::recompute);
        connect(m_model, &QAbstractItemModel::modelReset, this, &RegionWatcher::recompute);
        connect(m_model, &QAbstractItemModel::layoutChanged, this, &RegionWatcher::recompute);
    }
    Q_EMIT modelChanged();
    recompute();
}

void RegionWatcher::setRegion(const QRectF &r)
{
    if (m_region == r) return;
    m_region = r;
    Q_EMIT regionChanged();
    recompute();
}

int RegionWatcher::role(const char *name) const
{
    const auto names = m_model->roleNames();
    for (auto it = names.begin(); it != names.end(); ++it)
        if (it.value() == name) return it.key();
    return -1;
}

void RegionWatcher::recompute()
{
    bool occ = false, anyMax = false;
    if (m_model) {
        const int geo = role("Geometry"), min = role("IsMinimized"), win = role("IsWindow"), maxr = role("IsMaximized"), hidden = role("IsHidden");
        for (int i = 0; i < m_model->rowCount(); ++i) {
            const QModelIndex idx = m_model->index(i, 0);
            if (win >= 0 && !idx.data(win).toBool()) continue;
            if (min >= 0 && idx.data(min).toBool()) continue;
            if (hidden >= 0 && idx.data(hidden).toBool()) continue;
            if (maxr >= 0 && idx.data(maxr).toBool()) anyMax = true;
            if (geo >= 0 && idx.data(geo).toRect().intersects(m_region.toAlignedRect())) occ = true;
        }
    }
    if (occ != m_occupied || anyMax != m_anyMax) {
        m_occupied = occ;
        m_anyMax = anyMax;
        Q_EMIT occupiedChanged();
    }
}
}
