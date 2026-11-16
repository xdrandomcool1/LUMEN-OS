// SPDX-License-Identifier: GPL-3.0-or-later
#include "processmodel.h"

#include <QCollator>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <pwd.h>
#include <unistd.h>

namespace Lumen {

static QByteArray readAll(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

ProcessModel::ProcessModel(QObject *parent) : QAbstractListModel(parent)
{
    m_hz = ::sysconf(_SC_CLK_TCK);
    m_cpus = int(qMax(1L, ::sysconf(_SC_NPROCESSORS_ONLN)));
    m_timer.setInterval(2000);
    connect(&m_timer, &QTimer::timeout, this, &ProcessModel::refresh);
}

void ProcessModel::setActive(bool a)
{
    if (a == active()) return;
    if (a) { refresh(); m_timer.start(); } else m_timer.stop();
    Q_EMIT activeChanged();
}

bool ProcessModel::parseStat(const QByteArray &stat, ProcInfo &p)
{
    // pid (comm) state ppid ... ; comm may contain spaces and parentheses.
    const int open = stat.indexOf('('), close = stat.lastIndexOf(')');
    if (open < 0 || close < open) return false;
    p.pid = stat.left(open).trimmed().toInt();
    p.name = QString::fromUtf8(stat.mid(open + 1, close - open - 1));
    const QList<QByteArray> f = stat.mid(close + 2).split(' ');
    if (f.size() < 22) return false;
    p.state = f[0].isEmpty() ? '?' : f[0][0];
    p.ppid = f[1].toInt();
    p.ticks = f[11].toULongLong() + f[12].toULongLong(); // utime + stime
    p.threads = f[17].toInt();
    p.rss = f[21].toLongLong() * ::sysconf(_SC_PAGESIZE);
    return true;
}

void ProcessModel::refresh()
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const double dt = m_lastMs ? (now - m_lastMs) / 1000.0 : 0;
    QHash<int, ProcInfo> next;
    const QDir proc(QStringLiteral("/proc"));
    for (const QString &e : proc.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        bool ok = false;
        const int pid = e.toInt(&ok);
        if (!ok) continue;
        ProcInfo p;
        if (!parseStat(readAll(QStringLiteral("/proc/%1/stat").arg(pid)), p)) continue;
        const QByteArray status = readAll(QStringLiteral("/proc/%1/status").arg(pid));
        const int u = status.indexOf("\nUid:");
        if (u >= 0) p.uid = status.mid(u + 5, status.indexOf('\n', u + 5) - u - 5).simplified().split(' ').value(0).toUInt();
        if (!m_users.contains(p.uid)) {
            passwd *pw = ::getpwuid(p.uid);
            m_users.insert(p.uid, pw ? QString::fromLocal8Bit(pw->pw_name) : QString::number(p.uid));
        }
        p.user = m_users.value(p.uid);
        QByteArray cmd = readAll(QStringLiteral("/proc/%1/cmdline").arg(pid));
        cmd.replace('\0', ' ');
        p.command = QString::fromUtf8(cmd).trimmed();
        // Prefer the executable's base name for GUI-friendly display.
        if (!p.command.isEmpty()) {
            const QString exe = p.command.section(QLatin1Char(' '), 0, 0).section(QLatin1Char('/'), -1);
            if (!exe.isEmpty() && exe.size() > p.name.size() && exe.startsWith(p.name)) p.name = exe;
        }
        const QByteArray io = readAll(QStringLiteral("/proc/%1/io").arg(pid));
        for (const QByteArray &l : io.split('\n')) {
            if (l.startsWith("read_bytes:")) p.readBytes = l.mid(11).trimmed().toLongLong();
            else if (l.startsWith("write_bytes:")) p.writeBytes = l.mid(12).trimmed().toLongLong();
        }
        const auto prev = m_procs.constFind(pid);
        if (prev != m_procs.constEnd() && dt > 0) {
            p.cpu = 100.0 * double(p.ticks - qMin(p.ticks, prev->ticks)) / m_hz / dt;
            p.readRate = qMax<qint64>(0, p.readBytes - prev->readBytes) / dt;
            p.writeRate = qMax<qint64>(0, p.writeBytes - prev->writeBytes) / dt;
        }
        next.insert(pid, p);
    }
    m_procs = std::move(next);
    m_lastMs = now;
    rebuild();
    Q_EMIT refreshed();
}

int ProcessModel::threadCount() const
{
    int n = 0;
    for (const auto &p : m_procs) n += p.threads;
    return n;
}

void ProcessModel::rebuild()
{
    QList<int> rows;
    const uint me = ::getuid();
    for (auto it = m_procs.cbegin(); it != m_procs.cend(); ++it) {
        if (m_onlyMine && it->uid != me) continue;
        if (!m_filter.isEmpty() && !it->name.contains(m_filter, Qt::CaseInsensitive)
            && !QString::number(it->pid).startsWith(m_filter)) continue;
        rows << it.key();
    }
    static QCollator c = [] { QCollator x; x.setCaseSensitivity(Qt::CaseInsensitive); return x; }();
    auto key = [&](const ProcInfo &p) -> double {
        if (m_sortKey == QLatin1String("memory")) return double(p.rss);
        if (m_sortKey == QLatin1String("threads")) return p.threads;
        if (m_sortKey == QLatin1String("pid")) return p.pid;
        if (m_sortKey == QLatin1String("read")) return p.readRate;
        if (m_sortKey == QLatin1String("write")) return p.writeRate;
        return p.cpu;
    };
    const bool textual = m_sortKey == QLatin1String("name") || m_sortKey == QLatin1String("user");
    std::stable_sort(rows.begin(), rows.end(), [&](int a, int b) {
        const ProcInfo &pa = m_procs[a], &pb = m_procs[b];
        if (textual) {
            const int r = m_sortKey == QLatin1String("name") ? c.compare(pa.name, pb.name) : c.compare(pa.user, pb.user);
            return m_desc ? r > 0 : r < 0;
        }
        const double ka = key(pa), kb = key(pb);
        if (ka == kb) return pa.pid < pb.pid;
        return m_desc ? ka > kb : ka < kb;
    });
    // Keep row identity where possible to avoid list jumps: reset is cheap
    // at this size (hundreds of rows) and keeps the code simple.
    beginResetModel();
    m_rows = rows;
    endResetModel();
}

QVariant ProcessModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_rows.size()) return {};
    const ProcInfo &p = m_procs[m_rows[index.row()]];
    switch (role) {
    case PidRole: return p.pid;
    case NameRole: return p.name;
    case CommandRole: return p.command;
    case UserRole: return p.user;
    case CpuRole: return p.cpu;
    case MemoryRole: return p.rss;
    case MemoryTextRole: {
        const double mb = p.rss / 1e6;
        return mb >= 1000 ? QString::number(mb / 1000, 'f', 2) + QStringLiteral(" GB") : QString::number(mb, 'f', 1) + QStringLiteral(" MB");
    }
    case ThreadsRole: return p.threads;
    case ReadRole: return p.readRate;
    case WriteRole: return p.writeRate;
    case StateRole: return QString(QLatin1Char(p.state));
    case IsMineRole: return p.uid == ::getuid();
    }
    return {};
}

QHash<int, QByteArray> ProcessModel::roleNames() const
{
    return {{PidRole, "pid"}, {NameRole, "name"}, {CommandRole, "command"}, {UserRole, "user"}, {CpuRole, "cpu"},
            {MemoryRole, "memory"}, {MemoryTextRole, "memoryText"}, {ThreadsRole, "threads"}, {ReadRole, "readRate"},
            {WriteRole, "writeRate"}, {StateRole, "state"}, {IsMineRole, "isMine"}};
}

QString ProcessModel::sendSignal(int pid, bool force)
{
    if (pid <= 1) return tr("This process cannot be quit.");
    if (::kill(pid, force ? SIGKILL : SIGTERM) != 0)
        return errno == EPERM ? tr("You don't have permission to quit this process.") : QString::fromLocal8Bit(std::strerror(errno));
    QTimer::singleShot(400, this, &ProcessModel::refresh);
    return {};
}

QVariantMap ProcessModel::details(int pid) const
{
    const auto it = m_procs.constFind(pid);
    if (it == m_procs.constEnd()) return {};
    return {{QStringLiteral("pid"), it->pid}, {QStringLiteral("ppid"), it->ppid}, {QStringLiteral("name"), it->name},
            {QStringLiteral("command"), it->command}, {QStringLiteral("user"), it->user}, {QStringLiteral("threads"), it->threads}};
}

// ---------------------------------------------------------------- GPU
GpuStats::GpuStats(QObject *parent) : QObject(parent)
{
    m_timer.setInterval(2000);
    connect(&m_timer, &QTimer::timeout, this, &GpuStats::sample);
    const QDir drm(QStringLiteral("/sys/class/drm"));
    for (const QString &c : drm.entryList({QStringLiteral("card?")}, QDir::Dirs | QDir::System)) {
        const QString busy = drm.filePath(c) + QStringLiteral("/device/gpu_busy_percent"); // amdgpu
        if (QFile::exists(busy)) { m_busyPath = busy; m_source = QStringLiteral("amdgpu"); m_available = true; break; }
    }
    if (!m_available && !QStandardPaths::findExecutable(QStringLiteral("nvidia-smi")).isEmpty()) {
        m_source = QStringLiteral("nvidia-smi");
        m_available = true;
    }
}

void GpuStats::sample()
{
    if (m_source == QLatin1String("amdgpu")) {
        m_usage = readAll(m_busyPath).trimmed().toDouble() / 100.0;
    } else if (m_source == QLatin1String("nvidia-smi")) {
        QProcess p;
        p.start(QStringLiteral("nvidia-smi"), {QStringLiteral("--query-gpu=utilization.gpu,name"), QStringLiteral("--format=csv,noheader,nounits")});
        if (p.waitForFinished(1500)) {
            const QString out = QString::fromUtf8(p.readAllStandardOutput()).trimmed();
            m_usage = out.section(QLatin1Char(','), 0, 0).trimmed().toDouble() / 100.0;
            m_name = out.section(QLatin1Char(','), 1).trimmed();
        }
    }
    Q_EMIT updated();
}
}
