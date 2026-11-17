// SPDX-License-Identifier: GPL-3.0-or-later
// App Store — Flatpak back-end (flatpak CLI, run asynchronously).
#pragma once
#include <QObject>
#include <QProcess>
#include <QQueue>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

namespace Lumen {
class Flatpak : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(QVariantList results READ results NOTIFY resultsChanged)
    Q_PROPERTY(QVariantList installed READ installed NOTIFY installedChanged)
    Q_PROPERTY(QVariantList updates READ updates NOTIFY updatesChanged)
    Q_PROPERTY(bool searching READ searching NOTIFY resultsChanged)
    Q_PROPERTY(QString busyApp READ busyApp NOTIFY progressChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString status READ status NOTIFY progressChanged)
public:
    explicit Flatpak(QObject *parent = nullptr);
    bool available() const;
    QVariantList results() const { return m_results; }
    QVariantList installed() const { return m_installed; }
    QVariantList updates() const { return m_updates; }
    bool searching() const { return m_searching; }
    QString busyApp() const { return m_busyApp; }
    int progress() const { return m_progress; }
    QString status() const { return m_status; }

    Q_INVOKABLE void search(const QString &query);
    Q_INVOKABLE void refreshInstalled();
    Q_INVOKABLE void refreshUpdates();
    Q_INVOKABLE void install(const QString &appId, const QString &remote = QStringLiteral("flathub"));
    Q_INVOKABLE void uninstall(const QString &appId);
    Q_INVOKABLE void updateAll();
    Q_INVOKABLE bool isInstalled(const QString &appId) const;
    Q_INVOKABLE void launch(const QString &appId);
    Q_INVOKABLE QString iconFor(const QString &appId) const;

    //! Parses `flatpak … --columns=…` tab-separated output into maps.
    static QVariantList parseColumns(const QString &out, const QStringList &keys);
    //! Extracts the last percentage from flatpak's progress output.
    static int parseProgress(const QString &chunk);
Q_SIGNALS:
    void resultsChanged();
    void installedChanged();
    void updatesChanged();
    void progressChanged();
    void operationFinished(const QString &appId, bool ok, const QString &message);
private:
    void runQueued();
    struct Op { QStringList args; QString appId; QString label; };
    QQueue<Op> m_queue;
    QProcess *m_op = nullptr;
    QProcess *m_search = nullptr;
    QVariantList m_results, m_installed, m_updates;
    bool m_searching = false;
    QString m_busyApp, m_status;
    int m_progress = 0;
};
}
