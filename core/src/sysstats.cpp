// SPDX-License-Identifier: GPL-3.0-or-later
#include "sysstats.h"
#include <QDateTime>
#include <QFile>
#include <QStorageInfo>

namespace Lumen {
SysStats::SysStats(QObject *parent) : QObject(parent)
{
    m_timer.setInterval(2000);
    connect(&m_timer, &QTimer::timeout, this, &SysStats::sample);
}

void SysStats::setActive(bool a)
{
    if (a == active()) return;
    if (a) { sample(); m_timer.start(); } else m_timer.stop();
    Q_EMIT activeChanged();
}

bool SysStats::parseCpu(const QByteArray &line, quint64 &idle, quint64 &total)
{
    const auto parts = line.simplified().split(' ');
    if (parts.size() < 5 || parts[0] != "cpu") return false;
    total = 0;
    for (int i = 1; i < parts.size(); ++i) total += parts[i].toULongLong();
    idle = parts[4].toULongLong() + (parts.size() > 5 ? parts[5].toULongLong() : 0); // idle + iowait
    return true;
}

QString SysStats::formatBytes(qint64 b)
{
    const char *u[] = {"B", "KB", "MB", "GB", "TB"};
    double v = b; int i = 0;
    while (v >= 1000 && i < 4) { v /= 1000; ++i; }
    return QString::number(v, 'f', v < 10 && i > 0 ? 1 : 0) + QLatin1Char(' ') + QLatin1String(u[i]);
}

void SysStats::sample()
{
    QFile stat(QStringLiteral("/proc/stat"));
    if (stat.open(QIODevice::ReadOnly)) {
        quint64 idle = 0, total = 0;
        if (parseCpu(stat.readLine(), idle, total) && m_prevTotal && total > m_prevTotal)
            m_cpu = 1.0 - qreal(idle - m_prevIdle) / qreal(total - m_prevTotal);
        m_prevIdle = idle; m_prevTotal = total;
    }
    QFile mem(QStringLiteral("/proc/meminfo"));
    if (mem.open(QIODevice::ReadOnly)) {
        qint64 total = 0, avail = 0;
        while (!mem.atEnd()) {
            const QByteArray l = mem.readLine();
            if (l.startsWith("MemTotal:")) total = l.simplified().split(' ').value(1).toLongLong() * 1024;
            else if (l.startsWith("MemAvailable:")) avail = l.simplified().split(' ').value(1).toLongLong() * 1024;
        }
        m_memTotal = total; m_memUsed = total - avail;
    }
    QFile net(QStringLiteral("/proc/net/dev"));
    if (net.open(QIODevice::ReadOnly)) {
        quint64 rx = 0, tx = 0;
        while (!net.atEnd()) {
            const QByteArray l = net.readLine();
            const int c = l.indexOf(':');
            if (c < 0) continue;
            const QByteArray ifname = l.left(c).trimmed();
            if (ifname == "lo") continue;
            const auto f = l.mid(c + 1).simplified().split(' ');
            if (f.size() >= 9) { rx += f[0].toULongLong(); tx += f[8].toULongLong(); }
        }
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (m_prevMs && now > m_prevMs) {
            const double dt = (now - m_prevMs) / 1000.0;
            m_down = rx >= m_prevRx ? (rx - m_prevRx) / dt : 0;
            m_up = tx >= m_prevTx ? (tx - m_prevTx) / dt : 0;
        }
        m_prevRx = rx; m_prevTx = tx; m_prevMs = now;
    }
    const QStorageInfo root(QStringLiteral("/"));
    if (root.isValid() && root.bytesTotal() > 0)
        m_disk = 1.0 - qreal(root.bytesAvailable()) / root.bytesTotal();
    m_history.append(m_cpu);
    while (m_history.size() > 30) m_history.removeFirst();
    Q_EMIT updated();
}
}
