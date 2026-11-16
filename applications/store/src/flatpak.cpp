// SPDX-License-Identifier: GPL-3.0-or-later
#include "flatpak.h"
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QStandardPaths>

namespace Lumen {

Flatpak::Flatpak(QObject *parent) : QObject(parent)
{
    refreshInstalled();
}

bool Flatpak::available() const { return !QStandardPaths::findExecutable(QStringLiteral("flatpak")).isEmpty(); }

QVariantList Flatpak::parseColumns(const QString &out, const QStringList &keys)
{
    QVariantList list;
    for (const QString &line : out.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
        const QStringList f = line.split(QLatin1Char('\t'));
        if (f.size() < 2) continue;
        QVariantMap m;
        for (int i = 0; i < keys.size(); ++i) m.insert(keys[i], f.value(i).trimmed());
        list.append(m);
    }
    return list;
}

int Flatpak::parseProgress(const QString &chunk)
{
    static const QRegularExpression re(QStringLiteral("(\\d{1,3})%"));
    int last = -1;
    auto it = re.globalMatch(chunk);
    while (it.hasNext()) last = it.next().captured(1).toInt();
    return last;
}

QString Flatpak::iconFor(const QString &appId) const
{
    // AppStream icon caches (system and user installations), then exports.
    const QStringList roots{QStringLiteral("/var/lib/flatpak"), QDir::homePath() + QStringLiteral("/.local/share/flatpak")};
    for (const QString &root : roots) {
        for (const char *size : {"128x128", "64x64"}) {
            const QString p = QStringLiteral("%1/appstream/flathub/x86_64/active/icons/%2/%3.png").arg(root, QLatin1String(size), appId);
            if (QFile::exists(p)) return QStringLiteral("file://") + p;
        }
        const QString exp = QStringLiteral("%1/exports/share/icons/hicolor/128x128/apps/%2.png").arg(root, appId);
        if (QFile::exists(exp)) return QStringLiteral("file://") + exp;
        const QString svg = QStringLiteral("%1/exports/share/icons/hicolor/scalable/apps/%2.svg").arg(root, appId);
        if (QFile::exists(svg)) return QStringLiteral("file://") + svg;
    }
    return {};
}

void Flatpak::search(const QString &query)
{
    if (m_search) { m_search->kill(); m_search->deleteLater(); }
    if (query.trimmed().size() < 2) { m_results.clear(); m_searching = false; Q_EMIT resultsChanged(); return; }
    m_search = new QProcess(this);
    m_searching = true;
    Q_EMIT resultsChanged();
    QProcess *p = m_search;
    connect(p, &QProcess::finished, this, [this, p] {
        if (p != m_search) return;
        QVariantList list = parseColumns(QString::fromUtf8(p->readAllStandardOutput()),
                                         {QStringLiteral("name"), QStringLiteral("description"), QStringLiteral("appId"),
                                          QStringLiteral("version"), QStringLiteral("branch"), QStringLiteral("remotes")});
        for (auto &v : list) {
            QVariantMap m = v.toMap();
            m.insert(QStringLiteral("icon"), iconFor(m.value(QStringLiteral("appId")).toString()));
            m.insert(QStringLiteral("installed"), isInstalled(m.value(QStringLiteral("appId")).toString()));
            v = m;
        }
        m_results = list;
        m_searching = false;
        Q_EMIT resultsChanged();
        m_search = nullptr;
        p->deleteLater();
    });
    p->start(QStringLiteral("flatpak"), {QStringLiteral("search"), QStringLiteral("--columns=name,description,application,version,branch,remotes"), query});
}

void Flatpak::refreshInstalled()
{
    auto *p = new QProcess(this);
    connect(p, &QProcess::finished, this, [this, p] {
        m_installed = parseColumns(QString::fromUtf8(p->readAllStandardOutput()),
                                   {QStringLiteral("name"), QStringLiteral("appId"), QStringLiteral("version"), QStringLiteral("size"), QStringLiteral("origin")});
        for (auto &v : m_installed) { QVariantMap m = v.toMap(); m.insert(QStringLiteral("icon"), iconFor(m.value(QStringLiteral("appId")).toString())); v = m; }
        Q_EMIT installedChanged();
        p->deleteLater();
    });
    p->start(QStringLiteral("flatpak"), {QStringLiteral("list"), QStringLiteral("--app"), QStringLiteral("--columns=name,application,version,size,origin")});
}

void Flatpak::refreshUpdates()
{
    auto *p = new QProcess(this);
    connect(p, &QProcess::finished, this, [this, p] {
        m_updates = parseColumns(QString::fromUtf8(p->readAllStandardOutput()),
                                 {QStringLiteral("name"), QStringLiteral("appId"), QStringLiteral("version")});
        Q_EMIT updatesChanged();
        p->deleteLater();
    });
    p->start(QStringLiteral("flatpak"), {QStringLiteral("remote-ls"), QStringLiteral("--updates"), QStringLiteral("--app"), QStringLiteral("--columns=name,application,version")});
}

bool Flatpak::isInstalled(const QString &appId) const
{
    for (const auto &v : m_installed)
        if (v.toMap().value(QStringLiteral("appId")) == appId) return true;
    return false;
}

void Flatpak::install(const QString &appId, const QString &remote)
{
    m_queue.enqueue({{QStringLiteral("install"), QStringLiteral("-y"), QStringLiteral("--noninteractive"), remote, appId}, appId, tr("Installing")});
    runQueued();
}

void Flatpak::uninstall(const QString &appId)
{
    m_queue.enqueue({{QStringLiteral("uninstall"), QStringLiteral("-y"), QStringLiteral("--noninteractive"), appId}, appId, tr("Removing")});
    runQueued();
}

void Flatpak::updateAll()
{
    m_queue.enqueue({{QStringLiteral("update"), QStringLiteral("-y"), QStringLiteral("--noninteractive")}, QStringLiteral("*"), tr("Updating")});
    runQueued();
}

void Flatpak::runQueued()
{
    if (m_op || m_queue.isEmpty()) return;
    const Op op = m_queue.dequeue();
    m_op = new QProcess(this);
    m_op->setProcessChannelMode(QProcess::MergedChannels);
    m_busyApp = op.appId;
    m_status = op.label;
    m_progress = 0;
    Q_EMIT progressChanged();
    connect(m_op, &QProcess::readyRead, this, [this] {
        const int p = parseProgress(QString::fromUtf8(m_op->readAll()));
        if (p >= 0 && p != m_progress) { m_progress = p; Q_EMIT progressChanged(); }
    });
    connect(m_op, &QProcess::finished, this, [this, op](int code) {
        const bool ok = code == 0;
        Q_EMIT operationFinished(op.appId, ok, ok ? QString() : tr("flatpak exited with code %1").arg(code));
        m_op->deleteLater();
        m_op = nullptr;
        m_busyApp.clear();
        m_progress = 0;
        Q_EMIT progressChanged();
        refreshInstalled();
        refreshUpdates();
        runQueued();
    });
    m_op->start(QStringLiteral("flatpak"), op.args);
}

void Flatpak::launch(const QString &appId)
{
    QProcess::startDetached(QStringLiteral("flatpak"), {QStringLiteral("run"), appId});
}
}
