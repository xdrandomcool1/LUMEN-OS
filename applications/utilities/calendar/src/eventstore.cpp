// SPDX-License-Identifier: GPL-3.0-or-later
#include "eventstore.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUuid>

namespace Lumen {
QString EventStore::path()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/lumen/calendar/events.json");
}

EventStore::EventStore(QObject *parent) : QObject(parent)
{
    QFile f(path());
    if (f.open(QIODevice::ReadOnly))
        m_events = QJsonDocument::fromJson(f.readAll()).array().toVariantList();
}

QVariantList EventStore::eventsOn(const QString &date) const
{
    QVariantList out;
    for (const auto &e : m_events)
        if (e.toMap().value(QStringLiteral("date")).toString() == date) out << e;
    std::sort(out.begin(), out.end(), [](const QVariant &a, const QVariant &b) {
        return a.toMap().value(QStringLiteral("time")).toString() < b.toMap().value(QStringLiteral("time")).toString();
    });
    return out;
}

void EventStore::add(const QString &date, const QString &title, const QString &time)
{
    m_events.append(QVariantMap{{QStringLiteral("id"), QUuid::createUuid().toString(QUuid::WithoutBraces)},
                                {QStringLiteral("date"), date}, {QStringLiteral("title"), title.trimmed()}, {QStringLiteral("time"), time.trimmed()}});
    save();
}

void EventStore::remove(const QString &id)
{
    m_events.removeIf([&](const QVariant &e) { return e.toMap().value(QStringLiteral("id")).toString() == id; });
    save();
}

void EventStore::save()
{
    QDir().mkpath(QFileInfo(path()).absolutePath());
    QSaveFile f(path()); // atomic replace
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(QJsonArray::fromVariantList(m_events)).toJson());
        f.commit();
    }
    ++m_rev;
    Q_EMIT changed();
}
}
