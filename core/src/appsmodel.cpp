// SPDX-License-Identifier: GPL-3.0-or-later
#include "appsmodel.h"

#include <KApplicationTrader>
#include <KIO/ApplicationLauncherJob>
#include <KIO/CommandLauncherJob>
#include <KIO/OpenUrlJob>
#include <KIO/CopyJob>
#include <KIO/EmptyTrashJob>
#include <KSycoca>
#include <QCollator>
#include <QQmlEngine>
#include <QUrl>

namespace Lumen {

AppsModel::AppsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    reload();
    connect(KSycoca::self(), &KSycoca::databaseChanged, this, &AppsModel::reload);
}

AppsModel *AppsModel::instance()
{
    static AppsModel *s = new AppsModel;
    return s;
}

AppsModel *AppsModel::create(QQmlEngine *, QJSEngine *)
{
    auto *m = instance();
    QQmlEngine::setObjectOwnership(m, QQmlEngine::CppOwnership);
    return m;
}

void AppsModel::reload()
{
    beginResetModel();
    m_apps = KApplicationTrader::query([](const KService::Ptr &s) {
        return !s->noDisplay() && s->showOnCurrentPlatform() && !s->exec().isEmpty();
    });
    QCollator c;
    c.setCaseSensitivity(Qt::CaseInsensitive);
    std::sort(m_apps.begin(), m_apps.end(), [&](const KService::Ptr &a, const KService::Ptr &b) {
        return c.compare(a->name(), b->name()) < 0;
    });
    endResetModel();
    Q_EMIT countChanged();
}

int AppsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_apps.size();
}

QVariant AppsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_apps.size())
        return {};
    const auto &s = m_apps.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case NameRole: return s->name();
    case GenericNameRole: return s->genericName();
    case IconRole: return s->icon();
    case DesktopIdRole: return s->desktopEntryName();
    case CommentRole: return s->comment();
    case CategoriesRole: return s->categories();
    case KeywordsRole: return s->keywords();
    }
    return {};
}

QHash<int, QByteArray> AppsModel::roleNames() const
{
    return {{NameRole, "name"}, {GenericNameRole, "genericName"}, {IconRole, "iconName"},
            {DesktopIdRole, "desktopId"}, {CommentRole, "comment"}, {CategoriesRole, "categories"},
            {KeywordsRole, "keywords"}};
}

int AppsModel::matchScore(const QString &query, const QString &name, const QString &generic,
                          const QStringList &keywords, const QString &exec)
{
    const QString q = query.trimmed().toLower();
    if (q.isEmpty())
        return 1; // Launchpad lists every app (stable alphabetical order)
    const QString n = name.toLower();
    int score = 0;
    if (n == q) score = 1000;
    else if (n.startsWith(q)) score = 800 - qMin(n.size() - q.size(), 100);
    else {
        // Word-prefix match ("store" matches "Lumen Store").
        const auto words = n.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        for (const auto &w : words)
            if (w.startsWith(q)) { score = qMax(score, 650); break; }
        // Initials ("am" → Activity Monitor).
        QString initials;
        for (const auto &w : words) initials += w.at(0);
        if (q.size() >= 2 && initials.startsWith(q)) score = qMax(score, 600);
        if (!score && n.contains(q)) score = 500;
        // Subsequence (fuzzy) match, penalised by gaps.
        if (!score) {
            int pos = 0, gaps = 0, last = -1;
            for (const QChar ch : q) {
                const int i = n.indexOf(ch, pos);
                if (i < 0) { pos = -1; break; }
                if (last >= 0 && i > last + 1) gaps += i - last - 1;
                last = i; pos = i + 1;
            }
            if (pos >= 0 && q.size() >= 2) score = qMax(0, 300 - gaps * 15);
        }
    }
    if (score < 400 && generic.toLower().contains(q)) score = qMax(score, 420);
    if (score < 400) {
        for (const auto &k : keywords)
            if (k.toLower().startsWith(q)) { score = qMax(score, 400); break; }
    }
    if (score < 200 && exec.toLower().startsWith(q)) score = qMax(score, 250);
    return score;
}

QVariantList AppsModel::search(const QString &query, int limit) const
{
    struct Hit { int score; const KService::Ptr *svc; };
    QList<Hit> hits;
    for (const auto &s : m_apps) {
        const int sc = matchScore(query, s->name(), s->genericName(), s->keywords(),
                                  s->exec().section(QLatin1Char(' '), 0, 0).section(QLatin1Char('/'), -1));
        if (sc > 0)
            hits.append({sc, &s});
    }
    std::stable_sort(hits.begin(), hits.end(), [](const Hit &a, const Hit &b) { return a.score > b.score; });
    QVariantList out;
    for (int i = 0; i < hits.size() && i < limit; ++i) {
        const auto &s = *hits[i].svc;
        out.append(QVariantMap{{QStringLiteral("name"), s->name()},
                               {QStringLiteral("genericName"), s->genericName()},
                               {QStringLiteral("iconName"), s->icon()},
                               {QStringLiteral("desktopId"), s->desktopEntryName()},
                               {QStringLiteral("comment"), s->comment()},
                               {QStringLiteral("score"), hits[i].score}});
    }
    return out;
}

QVariantMap AppsModel::app(const QString &desktopId) const
{
    KService::Ptr s = KService::serviceByDesktopName(desktopId);
    if (!s)
        s = KService::serviceByStorageId(desktopId);
    if (!s)
        return {};
    return {{QStringLiteral("name"), s->name()}, {QStringLiteral("iconName"), s->icon()},
            {QStringLiteral("desktopId"), s->desktopEntryName()}, {QStringLiteral("comment"), s->comment()}};
}

bool AppsModel::launch(const QString &desktopId, const QStringList &urls)
{
    KService::Ptr s = KService::serviceByDesktopName(desktopId);
    if (!s)
        s = KService::serviceByStorageId(desktopId);
    if (!s)
        return false;
    auto *job = new KIO::ApplicationLauncherJob(s);
    QList<QUrl> list;
    for (const auto &u : urls)
        list << QUrl::fromUserInput(u);
    job->setUrls(list);
    job->start();
    Q_EMIT launched(s->desktopEntryName());
    return true;
}

bool AppsModel::openUrl(const QString &url)
{
    auto *job = new KIO::OpenUrlJob(QUrl::fromUserInput(url));
    job->start();
    return true;
}

void AppsModel::runCommand(const QString &command)
{
    auto *job = new KIO::CommandLauncherJob(command);
    job->start();
}

} // namespace Lumen

namespace Lumen {
void AppsModel::trashUrls(const QList<QUrl> &urls)
{
    if (!urls.isEmpty())
        KIO::trash(urls)->start();
}

void AppsModel::emptyTrash()
{
    KIO::emptyTrash()->start();
}
}
