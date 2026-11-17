// SPDX-License-Identifier: GPL-3.0-or-later
#include "gestures.h"

#include <QAction>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <effect/effecthandler.h>
#include <effect/globals.h>

namespace Lumen {

void GesturesEffect::invokeShortcut(const QString &name)
{
    // Trigger KWin's own global actions (Overview, Window View, …) so the
    // gesture behaves exactly like the keyboard shortcut.
    QDBusMessage m = QDBusMessage::createMethodCall(QStringLiteral("org.kde.kglobalaccel"), QStringLiteral("/component/kwin"),
                                                    QStringLiteral("org.kde.kglobalaccel.Component"), QStringLiteral("invokeShortcut"));
    m << name;
    QDBusConnection::sessionBus().asyncCall(m);
}

GesturesEffect::GesturesEffect()
{
    auto *up = new QAction(this);
    connect(up, &QAction::triggered, this, [] { invokeShortcut(QStringLiteral("Overview")); });
    KWin::effects->registerTouchpadSwipeShortcut(KWin::SwipeDirection::Up, 3, up);

    auto *down = new QAction(this);
    connect(down, &QAction::triggered, this, [] { invokeShortcut(QStringLiteral("ExposeClass")); });
    KWin::effects->registerTouchpadSwipeShortcut(KWin::SwipeDirection::Down, 3, down);

    auto *left = new QAction(this);
    connect(left, &QAction::triggered, this, [] {
        if (auto *d = KWin::effects->desktopToRight(nullptr, false)) KWin::effects->setCurrentDesktop(d);
    });
    KWin::effects->registerTouchpadSwipeShortcut(KWin::SwipeDirection::Left, 3, left);

    auto *right = new QAction(this);
    connect(right, &QAction::triggered, this, [] {
        if (auto *d = KWin::effects->desktopToLeft(nullptr, false)) KWin::effects->setCurrentDesktop(d);
    });
    KWin::effects->registerTouchpadSwipeShortcut(KWin::SwipeDirection::Right, 3, right);

    auto *pinch = new QAction(this);
    connect(pinch, &QAction::triggered, this, [] { invokeShortcut(QStringLiteral("Overview")); });
    KWin::effects->registerTouchpadPinchShortcut(KWin::PinchDirection::Contracting, 4, pinch);
}

} // namespace Lumen

KWIN_EFFECT_FACTORY_SUPPORTED_ENABLED(Lumen::GesturesEffect, "metadata.json", return Lumen::GesturesEffect::supported();, return Lumen::GesturesEffect::enabledByDefault();)

#include "gestures.moc"
