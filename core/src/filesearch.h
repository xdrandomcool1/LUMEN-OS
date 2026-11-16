// SPDX-License-Identifier: GPL-3.0-or-later
// File search for Spotlight. Uses the lumend index (org.lumen.Search) when the
// service runs; otherwise a bounded in-process index of the user's folders.
#pragma once
#include <QFutureWatcher>
#include <QMutex>
#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

namespace Lumen {
class FileSearch : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool indexing READ indexing NOTIFY indexingChanged)
    Q_PROPERTY(int indexedCount READ indexedCount NOTIFY indexingChanged)
public:
    explicit FileSearch(QObject *parent = nullptr);
    bool indexing() const { return m_indexing; }
    int indexedCount() const;
    //! [{name, folder, url, iconName}] ranked: prefix > word prefix > substring.
    Q_INVOKABLE QVariantList search(const QString &query, int limit = 8);
    Q_INVOKABLE void reindex();

    //! Exposed for tests.
    static QStringList scan(const QStringList &roots, int maxDepth, int maxFiles);
    static QVariantList rank(const QStringList &paths, const QString &query, int limit);
Q_SIGNALS:
    void indexingChanged();
private:
    QVariantList searchDaemon(const QString &query, int limit, bool *ok);
    mutable QMutex m_mutex;
    QStringList m_paths;
    bool m_indexing = false;
    QFutureWatcher<QStringList> m_watcher;
};
}
