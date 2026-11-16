// SPDX-License-Identifier: GPL-3.0-or-later
#include "keyboardlayout.h"
#include <QDBusPendingCall>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>

namespace Lumen {
static const QString kSvc = QStringLiteral("org.kde.keyboard");
static const QString kPath = QStringLiteral("/Layouts");
static const QString kIface = QStringLiteral("org.kde.KeyboardLayouts");

KeyboardLayout::KeyboardLayout(QObject *parent) : QObject(parent)
{
    auto bus = QDBusConnection::sessionBus();
    bus.connect(kSvc, kPath, kIface, QStringLiteral("layoutChanged"), this, SLOT(onLayoutChanged(uint)));
    bus.connect(kSvc, kPath, kIface, QStringLiteral("layoutListChanged"), this, SLOT(refresh()));
    refresh();
}

void KeyboardLayout::refresh()
{
    QDBusMessage m = QDBusMessage::createMethodCall(kSvc, kPath, kIface, QStringLiteral("getLayoutsList"));
    QDBusMessage r = QDBusConnection::sessionBus().call(m, QDBus::Block, 1000);
    m_layouts.clear();
    if (r.type() == QDBusMessage::ReplyMessage && !r.arguments().isEmpty()) {
        const QDBusArgument arg = r.arguments().first().value<QDBusArgument>();
        arg.beginArray();
        while (!arg.atEnd()) {
            QString shortName, variant, longName;
            arg.beginStructure();
            arg >> shortName >> variant >> longName;
            arg.endStructure();
            m_layouts.append(QVariantMap{{QStringLiteral("shortName"), shortName}, {QStringLiteral("variant"), variant}, {QStringLiteral("name"), longName}});
        }
        arg.endArray();
    }
    QDBusInterface i(kSvc, kPath, kIface);
    QDBusReply<uint> cur = i.call(QStringLiteral("getLayout"));
    m_current = cur.isValid() ? int(cur.value()) : 0;
    Q_EMIT changed();
}

void KeyboardLayout::onLayoutChanged(uint index)
{
    m_current = int(index);
    Q_EMIT changed();
}

void KeyboardLayout::setCurrent(int i)
{
    QDBusInterface iface(kSvc, kPath, kIface);
    iface.asyncCall(QStringLiteral("setLayout"), uint(i));
}

void KeyboardLayout::next()
{
    QDBusInterface iface(kSvc, kPath, kIface);
    iface.asyncCall(QStringLiteral("switchToNextLayout"));
}

QString KeyboardLayout::shortName() const
{
    if (m_current < 0 || m_current >= m_layouts.size())
        return {};
    return m_layouts.at(m_current).toMap().value(QStringLiteral("shortName")).toString().toUpper();
}
}
