// SPDX-License-Identifier: GPL-3.0-or-later
#include "foldermodel.h"

#include <KApplicationTrader>
#include <KDirLister>
#include <KDirModel>
#include <KFileItem>
#include <KIO/ApplicationLauncherJob>
#include <KIO/CopyJob>
#include <KIO/DeleteJob>
#include <KIO/EmptyTrashJob>
#include <KIO/Job>
#include <KIO/MkdirJob>
#include <KIO/OpenUrlJob>
#include <KIO/RestoreJob>
#include <KIO/SimpleJob>
#include <KUrlMimeData>
#include <KIO/Paste>
#include <KJob>
#include <QClipboard>
#include <QCollator>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QLocale>
#include <QMimeData>
#include <QProcess>
#include <QStandardPaths>

namespace Lumen {

FolderModel::FolderModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_dirModel(new KDirModel(this))
{
    m_dirModel->dirLister()->setAutoErrorHandlingEnabled(false);
    setSourceModel(m_dirModel);
    setDynamicSortFilter(true);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    sort(0);
    auto *lister = m_dirModel->dirLister();
    connect(lister, &KCoreDirLister::started, this, [this] { m_busy = true; Q_EMIT busyChanged(); });
    connect(lister, qOverload<>(&KCoreDirLister::completed), this, [this] { m_busy = false; Q_EMIT busyChanged(); Q_EMIT countChanged(); });
    connect(lister, &KCoreDirLister::jobError, this, [this](KIO::Job *job) { setError(job->errorString()); m_busy = false; Q_EMIT busyChanged(); });
    connect(this, &QAbstractItemModel::rowsInserted, this, &FolderModel::countChanged);
    connect(this, &QAbstractItemModel::rowsRemoved, this, &FolderModel::countChanged);
    connect(this, &QAbstractItemModel::modelReset, this, &FolderModel::countChanged);
}

FolderModel::~FolderModel() = default;

void FolderModel::setError(const QString &e)
{
    m_error = e;
    Q_EMIT errorOccurred(e);
}

void FolderModel::setUrl(const QUrl &url)
{
    QUrl u = url;
    if (u.scheme().isEmpty())
        u = QUrl::fromLocalFile(url.toString());
    if (m_url == u)
        return;
    m_url = u;
    m_filter.clear();
    Q_EMIT filterTextChanged();
    m_dirModel->dirLister()->setShowHiddenFiles(m_showHidden);
    m_dirModel->openUrl(m_url, KDirModel::NoFlags);
    Q_EMIT urlChanged();
}

QString FolderModel::title() const
{
    if (isTrash())
        return tr("Trash");
    if (m_url.isLocalFile()) {
        const QString p = m_url.toLocalFile();
        if (p == QDir::homePath())
            return QDir::home().dirName();
        if (p == QLatin1String("/"))
            return tr("Computer");
        return QFileInfo(p).fileName();
    }
    return m_url.fileName().isEmpty() ? m_url.toDisplayString() : m_url.fileName();
}

bool FolderModel::canGoUp() const
{
    return !isTrash() && m_url.path() != QLatin1String("/");
}

QUrl FolderModel::parentUrl() const
{
    return KIO::upUrl(m_url);
}

void FolderModel::setShowHidden(bool s)
{
    if (m_showHidden == s)
        return;
    m_showHidden = s;
    m_dirModel->dirLister()->setShowHiddenFiles(s);
    m_dirModel->dirLister()->emitChanges();
    invalidateFilter();
    Q_EMIT showHiddenChanged();
}

void FolderModel::setSortKey(const QString &k)
{
    if (m_sortKey == k)
        return;
    m_sortKey = k;
    invalidate();
    sort(0, m_ascending ? Qt::AscendingOrder : Qt::DescendingOrder);
    Q_EMIT sortChanged();
}

void FolderModel::setAscending(bool a)
{
    if (m_ascending == a)
        return;
    m_ascending = a;
    sort(0, a ? Qt::AscendingOrder : Qt::DescendingOrder);
    Q_EMIT sortChanged();
}

void FolderModel::setFilterText(const QString &f)
{
    if (m_filter == f)
        return;
    m_filter = f;
    invalidateFilter();
    Q_EMIT filterTextChanged();
    Q_EMIT countChanged();
}

void FolderModel::reload()
{
    m_dirModel->dirLister()->openUrl(m_url, KDirLister::Reload);
}

bool FolderModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex idx = m_dirModel->index(sourceRow, 0, sourceParent);
    const KFileItem item = m_dirModel->itemForIndex(idx);
    if (item.isNull())
        return false;
    if (!m_showHidden && item.isHidden())
        return false;
    if (!m_filter.isEmpty() && !item.text().contains(m_filter, Qt::CaseInsensitive))
        return false;
    return true;
}

bool FolderModel::lessThan(const QModelIndex &l, const QModelIndex &r) const
{
    const KFileItem a = m_dirModel->itemForIndex(l);
    const KFileItem b = m_dirModel->itemForIndex(r);
    // Folders first, like the reference file manager's default.
    if (a.isDir() != b.isDir())
        return m_ascending ? a.isDir() : b.isDir();
    static QCollator collator = [] { QCollator c; c.setNumericMode(true); c.setCaseSensitivity(Qt::CaseInsensitive); return c; }();
    if (m_sortKey == QLatin1String("modified")) {
        const auto ta = a.time(KFileItem::ModificationTime), tb = b.time(KFileItem::ModificationTime);
        if (ta != tb) return ta < tb;
    } else if (m_sortKey == QLatin1String("size")) {
        if (a.size() != b.size()) return a.size() < b.size();
    } else if (m_sortKey == QLatin1String("kind")) {
        const int c = collator.compare(a.mimeComment(), b.mimeComment());
        if (c != 0) return c < 0;
    }
    return collator.compare(a.text(), b.text()) < 0;
}

QHash<int, QByteArray> FolderModel::roleNames() const
{
    return {{NameRole, "name"}, {IconNameRole, "iconName"}, {UrlRole, "url"}, {PathRole, "path"},
            {IsDirRole, "isDir"}, {SizeRole, "size"}, {SizeTextRole, "sizeText"}, {ModifiedRole, "modified"},
            {ModifiedTextRole, "modifiedText"}, {KindRole, "kind"}, {MimeTypeRole, "mimeType"},
            {IsHiddenRole, "isHidden"}, {IsImageRole, "isImage"}, {IsLinkRole, "isLink"}};
}

QString FolderModel::formatSize(qint64 bytes)
{
    if (bytes < 1000)
        return tr("%n bytes", nullptr, int(bytes));
    const char *units[] = {"KB", "MB", "GB", "TB"};
    double v = bytes;
    int i = -1;
    while (v >= 1000 && i < 3) { v /= 1000; ++i; }
    return QLocale().toString(v, 'f', v < 10 ? 1 : 0) + QLatin1Char(' ') + QLatin1String(units[i]);
}

QVariant FolderModel::data(const QModelIndex &index, int role) const
{
    if (role < NameRole)
        return QSortFilterProxyModel::data(index, role);
    const KFileItem item = m_dirModel->itemForIndex(mapToSource(index));
    if (item.isNull())
        return {};
    switch (role) {
    case NameRole: return item.text();
    case IconNameRole: return item.iconName();
    case UrlRole: return item.url();
    case PathRole: return item.localPath();
    case IsDirRole: return item.isDir();
    case SizeRole: return item.isDir() ? -1 : qint64(item.size());
    case SizeTextRole: return item.isDir() ? QStringLiteral("--") : formatSize(item.size());
    case ModifiedRole: return item.time(KFileItem::ModificationTime);
    case ModifiedTextRole: {
        const QDateTime t = item.time(KFileItem::ModificationTime);
        const QDate today = QDate::currentDate();
        if (t.date() == today) return tr("Today at %1").arg(QLocale().toString(t.time(), QLocale::ShortFormat));
        if (t.date() == today.addDays(-1)) return tr("Yesterday at %1").arg(QLocale().toString(t.time(), QLocale::ShortFormat));
        return QLocale().toString(t, QStringLiteral("d MMM yyyy 'at' HH:mm"));
    }
    case KindRole: return item.isDir() ? tr("Folder") : item.mimeComment();
    case MimeTypeRole: return item.mimetype();
    case IsHiddenRole: return item.isHidden();
    case IsImageRole: return item.mimetype().startsWith(QLatin1String("image/"));
    case IsLinkRole: return item.isLink();
    }
    return {};
}

KFileItem FolderModel::itemAt(int row) const
{
    if (row < 0 || row >= rowCount())
        return {};
    return m_dirModel->itemForIndex(mapToSource(index(row, 0)));
}

QVariantMap FolderModel::get(int row) const
{
    QVariantMap m;
    const auto names = roleNames();
    for (auto it = names.begin(); it != names.end(); ++it)
        m.insert(QString::fromLatin1(it.value()), data(index(row, 0), it.key()));
    return m;
}

QUrl FolderModel::urlAt(int row) const { return itemAt(row).url(); }
bool FolderModel::isDirAt(int row) const { return itemAt(row).isDir(); }

QList<QUrl> FolderModel::urlsAt(const QList<int> &rows) const
{
    QList<QUrl> out;
    for (int r : rows)
        if (const KFileItem i = itemAt(r); !i.isNull())
            out << i.url();
    return out;
}

void FolderModel::watchJob(KJob *job)
{
    connect(job, &KJob::result, this, [this](KJob *j) {
        if (j->error())
            setError(j->errorString());
    });
}

void FolderModel::open(int row)
{
    const KFileItem item = itemAt(row);
    if (item.isNull())
        return;
    if (item.isDir()) {
        Q_EMIT navigateRequested(item.url());
        return;
    }
    if (isTrash())
        return; // items must be put back before opening
    auto *job = new KIO::OpenUrlJob(item.targetUrl(), item.mimetype());
    job->setRunExecutables(false);
    watchJob(job);
    job->start();
}

QVariantList FolderModel::openWithApps(int row) const
{
    QVariantList out;
    const KFileItem item = itemAt(row);
    if (item.isNull())
        return out;
    for (const auto &s : KApplicationTrader::queryByMimeType(item.mimetype()))
        out.append(QVariantMap{{QStringLiteral("name"), s->name()}, {QStringLiteral("iconName"), s->icon()},
                               {QStringLiteral("desktopId"), s->storageId()}});
    return out;
}

void FolderModel::openWithApp(int row, const QString &desktopId)
{
    const KFileItem item = itemAt(row);
    KService::Ptr s = KService::serviceByStorageId(desktopId);
    if (item.isNull() || !s)
        return;
    auto *job = new KIO::ApplicationLauncherJob(s);
    job->setUrls({item.targetUrl()});
    watchJob(job);
    job->start();
}

void FolderModel::rename(int row, const QString &newName)
{
    const KFileItem item = itemAt(row);
    const QString name = newName.trimmed();
    if (item.isNull() || name.isEmpty() || name == item.text() || name.contains(QLatin1Char('/')))
        return;
    QUrl dest = item.url().adjusted(QUrl::RemoveFilename);
    dest.setPath(dest.path() + name);
    auto *job = KIO::moveAs(item.url(), dest, KIO::HideProgressInfo);
    watchJob(job);
}

QString FolderModel::uniqueName(const QString &base) const
{
    if (!m_url.isLocalFile())
        return base;
    const QDir dir(m_url.toLocalFile());
    if (!dir.exists(base))
        return base;
    const QFileInfo fi(base);
    const QString stem = fi.completeBaseName().isEmpty() ? base : fi.completeBaseName();
    const QString ext = fi.suffix().isEmpty() ? QString() : QLatin1Char('.') + fi.suffix();
    for (int i = 2; i < 1000; ++i) {
        const QString n = QStringLiteral("%1 %2%3").arg(stem).arg(i).arg(ext);
        if (!dir.exists(n))
            return n;
    }
    return base;
}

void FolderModel::createFolder(const QString &name)
{
    QUrl dest = m_url;
    dest.setPath(QDir::cleanPath(dest.path() + QLatin1Char('/') + uniqueName(name.isEmpty() ? tr("untitled folder") : name)));
    watchJob(KIO::mkdir(dest));
}

void FolderModel::trash(const QList<int> &rows)
{
    const auto urls = urlsAt(rows);
    if (urls.isEmpty())
        return;
    if (isTrash()) {
        deletePermanently(rows);
        return;
    }
    watchJob(KIO::trash(urls));
}

void FolderModel::deletePermanently(const QList<int> &rows)
{
    const auto urls = urlsAt(rows);
    if (!urls.isEmpty())
        watchJob(KIO::del(urls));
}

void FolderModel::restore(const QList<int> &rows)
{
    const auto urls = urlsAt(rows);
    if (!urls.isEmpty())
        watchJob(KIO::restoreFromTrash(urls));
}

void FolderModel::emptyTrash()
{
    watchJob(KIO::emptyTrash());
}

void FolderModel::duplicate(const QList<int> &rows)
{
    for (int r : rows) {
        const KFileItem item = itemAt(r);
        if (item.isNull())
            continue;
        const QFileInfo fi(item.text());
        const QString base = item.isDir() ? item.text() + tr(" copy")
                                          : fi.completeBaseName() + tr(" copy") + (fi.suffix().isEmpty() ? QString() : QLatin1Char('.') + fi.suffix());
        QUrl dest = m_url;
        dest.setPath(QDir::cleanPath(dest.path() + QLatin1Char('/') + uniqueName(base)));
        watchJob(KIO::copyAs(item.url(), dest));
    }
}

void FolderModel::compress(const QList<int> &rows)
{
    if (!m_url.isLocalFile())
        return;
    QStringList names;
    for (int r : rows)
        if (const KFileItem i = itemAt(r); !i.isNull())
            names << i.text();
    if (names.isEmpty())
        return;
    const QString archive = uniqueName(names.size() == 1 ? names.first() + QStringLiteral(".zip") : tr("Archive.zip"));
    auto *p = new QProcess(this);
    p->setWorkingDirectory(m_url.toLocalFile());
    connect(p, &QProcess::finished, this, [this, p](int code) {
        if (code != 0)
            setError(tr("Compression failed: %1").arg(QString::fromLocal8Bit(p->readAllStandardError())));
        p->deleteLater();
    });
    p->start(QStringLiteral("zip"), QStringList{QStringLiteral("-r"), QStringLiteral("-q"), archive} + names);
}

void FolderModel::copyToClipboard(const QList<int> &rows, bool cut)
{
    const auto urls = urlsAt(rows);
    if (urls.isEmpty())
        return;
    auto *mime = new QMimeData;
    KUrlMimeData::setUrls(urls, {}, mime);
    KIO::setClipboardDataCut(mime, cut);
    m_cutPending = cut;
    QGuiApplication::clipboard()->setMimeData(mime);
}

bool FolderModel::canPaste() const
{
    const QMimeData *mime = QGuiApplication::clipboard()->mimeData();
    return mime && mime->hasUrls();
}

void FolderModel::paste()
{
    const QMimeData *mime = QGuiApplication::clipboard()->mimeData();
    if (!mime || !mime->hasUrls())
        return;
    const bool cut = KIO::isClipboardDataCut(mime);
    copyUrls(KUrlMimeData::urlsFromMimeData(mime), m_url, cut);
    if (cut)
        QGuiApplication::clipboard()->clear();
}

void FolderModel::copyUrls(const QList<QUrl> &urls, const QUrl &dest, bool move)
{
    if (urls.isEmpty())
        return;
    watchJob(move ? KIO::move(urls, dest) : KIO::copy(urls, dest));
}

QString FolderModel::previewText(int row) const
{
    const KFileItem item = itemAt(row);
    if (item.isNull() || !item.isLocalFile() || item.isDir())
        return {};
    const QString mt = item.mimetype();
    const bool texty = mt.startsWith(QLatin1String("text/")) || mt.contains(QLatin1String("json")) || mt.contains(QLatin1String("xml"))
        || mt.contains(QLatin1String("script")) || mt == QLatin1String("application/x-desktop");
    if (!texty)
        return {};
    QFile f(item.localPath());
    if (!f.open(QIODevice::ReadOnly))
        return {};
    return QString::fromUtf8(f.read(64 * 1024));
}

} // namespace Lumen
