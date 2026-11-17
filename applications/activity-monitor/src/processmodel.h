// SPDX-License-Identifier: GPL-3.0-or-later
// Activity Monitor — process table from /proc.
#pragma once
#include <QAbstractListModel>
#include <QHash>
#include <QTimer>
#include <QtQml/qqmlregistration.h>

namespace Lumen {

struct ProcInfo {
    int pid = 0;
    int ppid = 0;
    QString name, command, user;
    uint uid = 0;
    double cpu = 0;          // percent of one CPU
    qint64 rss = 0;          // bytes
    int threads = 0;
    qint64 readBytes = 0, writeBytes = 0;
    double readRate = 0, writeRate = 0;
    char state = '?';
    quint64 ticks = 0;
};

class ProcessModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString sortKey READ sortKey WRITE setSortKey NOTIFY sortChanged)
    Q_PROPERTY(bool descending READ descending WRITE setDescending NOTIFY sortChanged)
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(bool onlyMine READ onlyMine WRITE setOnlyMine NOTIFY filterChanged)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(int processCount READ processCount NOTIFY refreshed)
    Q_PROPERTY(int threadCount READ threadCount NOTIFY refreshed)
public:
    enum Roles { PidRole = Qt::UserRole + 1, NameRole, CommandRole, UserRole, CpuRole, MemoryRole, MemoryTextRole,
                 ThreadsRole, ReadRole, WriteRole, StateRole, IsMineRole };
    explicit ProcessModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &p = {}) const override { return p.isValid() ? 0 : m_rows.size(); }
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString sortKey() const { return m_sortKey; }
    void setSortKey(const QString &k) { if (k == m_sortKey) return; m_sortKey = k; Q_EMIT sortChanged(); rebuild(); }
    bool descending() const { return m_desc; }
    void setDescending(bool d) { if (d == m_desc) return; m_desc = d; Q_EMIT sortChanged(); rebuild(); }
    QString filter() const { return m_filter; }
    void setFilter(const QString &f) { if (f == m_filter) return; m_filter = f; Q_EMIT filterChanged(); rebuild(); }
    bool onlyMine() const { return m_onlyMine; }
    void setOnlyMine(bool m) { if (m == m_onlyMine) return; m_onlyMine = m; Q_EMIT filterChanged(); rebuild(); }
    bool active() const { return m_timer.isActive(); }
    void setActive(bool a);
    int processCount() const { return m_procs.size(); }
    int threadCount() const;

    Q_INVOKABLE void refresh();
    //! SIGTERM (quit) or SIGKILL (force quit); returns an error string or "".
    Q_INVOKABLE QString sendSignal(int pid, bool force);
    Q_INVOKABLE QVariantMap details(int pid) const;

    static bool parseStat(const QByteArray &stat, ProcInfo &p);
Q_SIGNALS:
    void sortChanged();
    void filterChanged();
    void activeChanged();
    void refreshed();
private:
    void rebuild();
    QTimer m_timer;
    QHash<int, ProcInfo> m_procs;
    QList<int> m_rows;
    QHash<uint, QString> m_users;
    quint64 m_lastTotal = 0;
    qint64 m_lastMs = 0;
    QString m_sortKey = QStringLiteral("cpu");
    bool m_desc = true;
    QString m_filter;
    bool m_onlyMine = false;
    long m_hz = 100;
    int m_cpus = 1;
};

class GpuStats : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool available READ available NOTIFY updated)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(qreal usage READ usage NOTIFY updated)
    Q_PROPERTY(QString source READ source CONSTANT)
    Q_PROPERTY(bool active READ active WRITE setActive)
public:
    explicit GpuStats(QObject *parent = nullptr);
    bool available() const { return m_available; }
    QString name() const { return m_name; }
    qreal usage() const { return m_usage; }
    QString source() const { return m_source; }
    bool active() const { return m_timer.isActive(); }
    void setActive(bool a) { if (a) { sample(); m_timer.start(); } else m_timer.stop(); }
Q_SIGNALS:
    void updated();
private:
    void sample();
    QTimer m_timer;
    QString m_busyPath, m_name, m_source;
    bool m_available = false;
    qreal m_usage = 0;
};
}
