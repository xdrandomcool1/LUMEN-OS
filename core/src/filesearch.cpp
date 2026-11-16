// SPDX-License-Identifier: GPL-3.0-or-later
#include "filesearch.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>
#include <QtConcurrent>

namespace Lumen {

FileSearch::FileSearch(QObject *parent) : QObject(parent)
{
    connect(&m_watcher, &QFutureWatcher<QStringList>::finished, this, [this] {
        {
            QMutexLocker l(&m_mutex);
            m_paths = m_watcher.result();
        }
        m_indexing = false;
        Q_EMIT indexingChanged();
    });
    // Index lazily a few seconds after login so it never delays startup.
    QTimer::singleShot(8000, this, &FileSearch::reindex);
    auto *t = new QTimer(this);
    t->setInterval(15 * 60 * 1000);
    connect(t, &QTimer::timeout, this, &FileSearch::reindex);
    t->start();
}

int FileSearch::indexedCount() const
{
    QMutexLocker l(&m_mutex);
    return m_paths.size();
}

QStringList FileSearch::scan(const QStringList &roots, int maxDepth, int maxFiles)
{
    QStringList out;
    for (const QString &root : roots) {
        const int baseDepth = QDir::cleanPath(root).count(QLatin1Char('/'));
        QDirIterator it(root, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext() && out.size() < maxFiles) {
            const QString p = it.next();
            const QFileInfo fi = it.fileInfo();
            if (fi.fileName().startsWith(QLatin1Char('.')))
                continue;
            if (p.count(QLatin1Char('/')) - baseDepth > maxDepth)
                continue;
            if (p.contains(QLatin1String("/node_modules/")) || p.contains(QLatin1String("/.git/")))
                continue;
            out << p;
        }
    }
    return out;
}

QVariantList FileSearch::rank(const QStringList &paths, const QString &query, int limit)
{
    const QString q = query.trimmed().toLower();
    if (q.size() < 2)
        return {};
    struct Hit { int score; QString path; };
    QList<Hit> hits;
    for (const QString &p : paths) {
        const QString name = p.section(QLatin1Char('/'), -1).toLower();
        int score = 0;
        if (name.startsWith(q)) score = 300;
        else if (name.contains(QLatin1Char(' ') + q) || name.contains(QLatin1Char('_') + q) || name.contains(QLatin1Char('-') + q)) score = 200;
        else if (name.contains(q)) score = 100;
        if (score) {
            score -= qMin(50, int(name.size() - q.size()));
            hits.append({score, p});
        }
    }
    std::stable_sort(hits.begin(), hits.end(), [](const Hit &a, const Hit &b) { return a.score > b.score; });
    QVariantList out;
    static QMimeDatabase db;
    const QString home = QDir::homePath();
    for (int i = 0; i < hits.size() && i < limit; ++i) {
        const QFileInfo fi(hits[i].path);
        QString folder = fi.absolutePath();
        if (folder.startsWith(home)) folder.replace(0, home.size(), QStringLiteral("~"));
        out.append(QVariantMap{{QStringLiteral("name"), fi.fileName()},
                               {QStringLiteral("folder"), folder},
                               {QStringLiteral("url"), QUrl::fromLocalFile(fi.absoluteFilePath()).toString()},
                               {QStringLiteral("iconName"), fi.isDir() ? QStringLiteral("folder") : db.mimeTypeForFile(fi, QMimeDatabase::MatchExtension).iconName()}});
    }
    return out;
}

void FileSearch::reindex()
{
    if (m_indexing)
        return;
    auto *iface = QDBusConnection::sessionBus().interface();
    if (iface && iface->isServiceRegistered(QStringLiteral("org.lumen.Search")))
        return; // lumend owns indexing
    m_indexing = true;
    Q_EMIT indexingChanged();
    QStringList roots;
    for (auto loc : {QStandardPaths::DesktopLocation, QStandardPaths::DocumentsLocation, QStandardPaths::DownloadLocation,
                     QStandardPaths::PicturesLocation, QStandardPaths::MusicLocation, QStandardPaths::MoviesLocation}) {
        const QString p = QStandardPaths::writableLocation(loc);
        if (QFileInfo(p).isDir() && p != QDir::homePath() && !roots.contains(p))
            roots << p;
    }
    m_watcher.setFuture(QtConcurrent::run([roots] { return scan(roots, 6, 60000); }));
}

QVariantList FileSearch::searchDaemon(const QString &query, int limit, bool *ok)
{
    *ok = false;
    auto *iface = QDBusConnection::sessionBus().interface();
    if (!iface || !iface->isServiceRegistered(QStringLiteral("org.lumen.Search")))
        return {};
    QDBusInterface s(QStringLiteral("org.lumen.Search"), QStringLiteral("/Search"), QStringLiteral("org.lumen.Search"));
    s.setTimeout(250);
    QDBusReply<QStringList> r = s.call(QStringLiteral("Query"), query, uint(limit * 4));
    if (!r.isValid())
        return {};
    *ok = true;
    return rank(r.value(), query, limit);
}

QVariantList FileSearch::search(const QString &query, int limit)
{
    bool ok = false;
    QVariantList res = searchDaemon(query, limit, &ok);
    if (ok)
        return res;
    QMutexLocker l(&m_mutex);
    return rank(m_paths, query, limit);
}
}
