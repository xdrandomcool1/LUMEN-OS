// SPDX-License-Identifier: GPL-3.0-or-later
// Calendar — local event storage ($XDG_DATA_HOME/lumen/calendar/events.json).
#pragma once
#include <QObject>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

namespace Lumen {
class EventStore : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int revision READ revision NOTIFY changed)
public:
    explicit EventStore(QObject *parent = nullptr);
    int revision() const { return m_rev; }
    Q_INVOKABLE QVariantList eventsOn(const QString &date) const;
    Q_INVOKABLE void add(const QString &date, const QString &title, const QString &time);
    Q_INVOKABLE void remove(const QString &id);
    static QString path();
Q_SIGNALS:
    void changed();
private:
    void save();
    QVariantList m_events;
    int m_rev = 0;
};
}
