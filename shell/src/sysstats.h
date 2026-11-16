// SPDX-License-Identifier: GPL-3.0-or-later
// Lightweight /proc sampler for widgets (polls only while `active`).
#pragma once
#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

namespace Lumen {
class SysStats : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(int interval READ interval WRITE setInterval NOTIFY activeChanged)
    Q_PROPERTY(qreal cpu READ cpu NOTIFY updated)
    Q_PROPERTY(qreal memory READ memory NOTIFY updated)
    Q_PROPERTY(qint64 memoryUsed READ memoryUsed NOTIFY updated)
    Q_PROPERTY(qint64 memoryTotal READ memoryTotal NOTIFY updated)
    Q_PROPERTY(qreal disk READ disk NOTIFY updated)
    Q_PROPERTY(qreal netDown READ netDown NOTIFY updated)
    Q_PROPERTY(qreal netUp READ netUp NOTIFY updated)
    Q_PROPERTY(QVariantList cpuHistory READ cpuHistory NOTIFY updated)
public:
    explicit SysStats(QObject *parent = nullptr);
    bool active() const { return m_timer.isActive(); }
    void setActive(bool a);
    int interval() const { return m_timer.interval(); }
    void setInterval(int ms) { m_timer.setInterval(ms); Q_EMIT activeChanged(); }
    qreal cpu() const { return m_cpu; }
    qreal memory() const { return m_memTotal ? qreal(m_memUsed) / m_memTotal : 0; }
    qint64 memoryUsed() const { return m_memUsed; }
    qint64 memoryTotal() const { return m_memTotal; }
    qreal disk() const { return m_disk; }
    qreal netDown() const { return m_down; }
    qreal netUp() const { return m_up; }
    QVariantList cpuHistory() const { return m_history; }

    //! Parses the aggregate "cpu" line of /proc/stat into (idle, total).
    static bool parseCpu(const QByteArray &line, quint64 &idle, quint64 &total);
    Q_INVOKABLE static QString formatBytes(qint64 bytes);
Q_SIGNALS:
    void activeChanged();
    void updated();
private:
    void sample();
    QTimer m_timer;
    quint64 m_prevIdle = 0, m_prevTotal = 0, m_prevRx = 0, m_prevTx = 0;
    qint64 m_prevMs = 0;
    qreal m_cpu = 0, m_disk = 0, m_down = 0, m_up = 0;
    qint64 m_memUsed = 0, m_memTotal = 0;
    QVariantList m_history;
};
}
