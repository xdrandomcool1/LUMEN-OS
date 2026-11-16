// SPDX-License-Identifier: GPL-3.0-or-later
#include "appmenu.h"

#include <dbusmenu-lxqt/dbusmenuimporter.h>
#include <QDBusPendingCall>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QMenu>
#include <QTimer>

namespace Lumen {

AppMenu::AppMenu(QObject *parent) : QObject(parent)
{
    // The registrar that apps export their menus to lives in kded's
    // "appmenu" module; make sure it is loaded.
    QDBusMessage m = QDBusMessage::createMethodCall(QStringLiteral("org.kde.kded6"), QStringLiteral("/kded"),
                                                    QStringLiteral("org.kde.kded6"), QStringLiteral("loadModule"));
    m << QStringLiteral("appmenu");
    QDBusConnection::sessionBus().asyncCall(m);
}

void AppMenu::setServiceName(const QString &s) { if (m_service != s) { m_service = s; Q_EMIT sourceChanged(); reimport(); } }
void AppMenu::setObjectPath(const QString &p) { if (m_path != p) { m_path = p; Q_EMIT sourceChanged(); reimport(); } }

void AppMenu::reimport()
{
    delete m_importer;
    m_importer = nullptr;
    m_menus.clear();
    Q_EMIT menusChanged();
    if (m_service.isEmpty() || m_path.isEmpty())
        return;
    m_importer = new DBusMenuImporter(m_service, m_path, ASYNCHRONOUS, this);
    connect(m_importer, &DBusMenuImporter::menuUpdated, this, [this] {
        // Ask the application to populate every top-level submenu (dbusmenu
        // fills submenus lazily on AboutToShow), then publish.
        if (QMenu *root = m_importer->menu()) {
            for (QAction *a : root->actions())
                if (a->menu())
                    Q_EMIT a->menu()->aboutToShow();
        }
        QTimer::singleShot(150, this, &AppMenu::rebuild);
    });
    m_importer->updateMenu();
}

void AppMenu::rebuild()
{
    m_menus.clear();
    if (m_importer && m_importer->menu()) {
        for (QAction *a : m_importer->menu()->actions()) {
            if (!a->isVisible() || !a->menu())
                continue;
            m_menus.append(QVariantMap{{QStringLiteral("title"), a->text().remove(QLatin1Char('&'))}});
        }
    }
    Q_EMIT menusChanged();
}

QVariantList AppMenu::describe(QMenu *menu, const QString &prefix, int depth) const
{
    QVariantList out;
    if (!menu)
        return out;
    const auto actions = menu->actions();
    for (int i = 0; i < actions.size(); ++i) {
        QAction *a = actions.at(i);
        if (!a->isVisible())
            continue;
        const QString path = prefix.isEmpty() ? QString::number(i) : prefix + QLatin1Char('/') + QString::number(i);
        if (a->isSeparator()) {
            out.append(QVariantMap{{QStringLiteral("separator"), true}});
            continue;
        }
        QVariantMap item{{QStringLiteral("text"), a->text().remove(QLatin1Char('&'))},
                         {QStringLiteral("shortcut"), a->shortcut().toString(QKeySequence::NativeText)},
                         {QStringLiteral("enabled"), a->isEnabled()},
                         {QStringLiteral("checkable"), a->isCheckable()},
                         {QStringLiteral("checked"), a->isChecked()},
                         {QStringLiteral("path"), path}};
        if (a->menu() && depth < 4) {
            Q_EMIT a->menu()->aboutToShow();
            item.insert(QStringLiteral("children"), describe(a->menu(), path, depth + 1));
        }
        out.append(item);
    }
    return out;
}

QVariantList AppMenu::items(int index) const
{
    if (!m_importer || !m_importer->menu())
        return {};
    int visibleIdx = -1;
    const auto actions = m_importer->menu()->actions();
    for (int i = 0; i < actions.size(); ++i) {
        QAction *a = actions.at(i);
        if (!a->isVisible() || !a->menu())
            continue;
        if (++visibleIdx == index)
            return describe(a->menu(), QString::number(i), 0);
    }
    return {};
}

QAction *AppMenu::actionAt(const QString &path) const
{
    if (!m_importer || !m_importer->menu())
        return nullptr;
    QMenu *menu = m_importer->menu();
    QAction *action = nullptr;
    for (const QString &part : path.split(QLatin1Char('/'))) {
        if (!menu) return nullptr;
        const auto actions = menu->actions();
        const int i = part.toInt();
        if (i < 0 || i >= actions.size()) return nullptr;
        action = actions.at(i);
        menu = action->menu();
    }
    return action;
}

void AppMenu::trigger(const QString &path)
{
    if (QAction *a = actionAt(path))
        a->trigger();
}
}
