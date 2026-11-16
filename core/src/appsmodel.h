// SPDX-License-Identifier: GPL-3.0-or-later
// lumen-shell — installed applications (XDG desktop entries) + launching.
#pragma once

#include <KService>
#include <QAbstractListModel>
#include <QUrl>
#include <QQmlEngine>
#include <QtQml/qqmlregistration.h>

namespace Lumen {

class AppsModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles { NameRole = Qt::UserRole + 1, GenericNameRole, IconRole, DesktopIdRole, CommentRole, CategoriesRole, KeywordsRole };
    Q_ENUM(Roles)

    explicit AppsModel(QObject *parent = nullptr);
    static AppsModel *instance();
    static AppsModel *create(QQmlEngine *, QJSEngine *);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    //! Ranked fuzzy search over name, generic name, keywords and exec.
    Q_INVOKABLE QVariantList search(const QString &query, int limit = 8) const;
    Q_INVOKABLE QVariantMap app(const QString &desktopId) const;
    Q_INVOKABLE bool launch(const QString &desktopId, const QStringList &urls = {});
    Q_INVOKABLE bool openUrl(const QString &url);
    Q_INVOKABLE void runCommand(const QString &command);
    Q_INVOKABLE void trashUrls(const QList<QUrl> &urls);
    Q_INVOKABLE void emptyTrash();

    //! Score used by search(); exposed for unit tests.
    static int matchScore(const QString &query, const QString &name, const QString &generic,
                          const QStringList &keywords, const QString &exec);

Q_SIGNALS:
    void countChanged();
    void launched(const QString &desktopId);

private:
    void reload();
    QList<KService::Ptr> m_apps;
};

} // namespace Lumen
