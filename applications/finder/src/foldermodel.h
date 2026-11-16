// SPDX-License-Identifier: GPL-3.0-or-later
// Lumen Finder — folder listing model (KIO KDirModel based).
#pragma once

#include <QDateTime>
#include <QSortFilterProxyModel>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

class KDirModel;
class KDirLister;
class KFileItem;
class KJob;

namespace Lumen {

class FolderModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QString title READ title NOTIFY urlChanged)
    Q_PROPERTY(bool showHidden READ showHidden WRITE setShowHidden NOTIFY showHiddenChanged)
    Q_PROPERTY(QString sortKey READ sortKey WRITE setSortKey NOTIFY sortChanged)
    Q_PROPERTY(bool ascending READ ascending WRITE setAscending NOTIFY sortChanged)
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool isTrash READ isTrash NOTIFY urlChanged)
    Q_PROPERTY(bool canGoUp READ canGoUp NOTIFY urlChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY errorOccurred)

public:
    enum Roles {
        NameRole = Qt::UserRole + 100, IconNameRole, UrlRole, PathRole, IsDirRole, SizeRole, SizeTextRole,
        ModifiedRole, ModifiedTextRole, KindRole, MimeTypeRole, IsHiddenRole, IsImageRole, IsLinkRole
    };

    explicit FolderModel(QObject *parent = nullptr);
    ~FolderModel() override;

    QUrl url() const { return m_url; }
    void setUrl(const QUrl &url);
    QString title() const;
    bool showHidden() const { return m_showHidden; }
    void setShowHidden(bool s);
    QString sortKey() const { return m_sortKey; }
    void setSortKey(const QString &k);
    bool ascending() const { return m_ascending; }
    void setAscending(bool a);
    QString filterText() const { return m_filter; }
    void setFilterText(const QString &f);
    bool busy() const { return m_busy; }
    int count() const { return rowCount(); }
    bool isTrash() const { return m_url.scheme() == QLatin1String("trash"); }
    bool canGoUp() const;
    QString lastError() const { return m_error; }

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;

    Q_INVOKABLE QVariantMap get(int row) const;
    Q_INVOKABLE QUrl urlAt(int row) const;
    Q_INVOKABLE QList<QUrl> urlsAt(const QList<int> &rows) const;
    Q_INVOKABLE bool isDirAt(int row) const;
    Q_INVOKABLE QUrl parentUrl() const;
    Q_INVOKABLE void reload();

    // Operations (KIO jobs; errors reported via errorOccurred)
    Q_INVOKABLE void open(int row);
    Q_INVOKABLE void openWithApp(int row, const QString &desktopId);
    Q_INVOKABLE void rename(int row, const QString &newName);
    Q_INVOKABLE void createFolder(const QString &name);
    Q_INVOKABLE void trash(const QList<int> &rows);
    Q_INVOKABLE void deletePermanently(const QList<int> &rows);
    Q_INVOKABLE void restore(const QList<int> &rows);
    Q_INVOKABLE void emptyTrash();
    Q_INVOKABLE void duplicate(const QList<int> &rows);
    Q_INVOKABLE void compress(const QList<int> &rows);
    Q_INVOKABLE void copyToClipboard(const QList<int> &rows, bool cut);
    Q_INVOKABLE void paste();
    Q_INVOKABLE bool canPaste() const;
    Q_INVOKABLE void copyUrls(const QList<QUrl> &urls, const QUrl &dest, bool move);
    Q_INVOKABLE QString uniqueName(const QString &base) const;
    Q_INVOKABLE QVariantList openWithApps(int row) const;
    //! First 64 KiB of a local text file (Quick Look); empty for other types.
    Q_INVOKABLE QString previewText(int row) const;

    //! Formats a size like the reference file manager (decimal units).
    Q_INVOKABLE static QString formatSize(qint64 bytes);

Q_SIGNALS:
    void urlChanged();
    void showHiddenChanged();
    void sortChanged();
    void filterTextChanged();
    void busyChanged();
    void countChanged();
    void errorOccurred(const QString &message);
    void navigateRequested(const QUrl &url);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    KFileItem itemAt(int row) const;
    void watchJob(KJob *job);
    void setError(const QString &e);

    KDirModel *m_dirModel;
    QUrl m_url;
    bool m_showHidden = false;
    QString m_sortKey = QStringLiteral("name");
    bool m_ascending = true;
    QString m_filter;
    bool m_busy = false;
    QString m_error;
    bool m_cutPending = false;
};

} // namespace Lumen
