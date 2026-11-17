// SPDX-License-Identifier: GPL-3.0-or-later
// Global application menu: imports the active window's DBusMenu
// (com.canonical.dbusmenu) and exposes it to the menu bar.
#pragma once
#include <QObject>
#include <QPointer>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

class DBusMenuImporter;
class QMenu;
class QAction;

namespace Lumen {
class AppMenu : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString serviceName READ serviceName WRITE setServiceName NOTIFY sourceChanged)
    Q_PROPERTY(QString objectPath READ objectPath WRITE setObjectPath NOTIFY sourceChanged)
    Q_PROPERTY(QVariantList menus READ menus NOTIFY menusChanged)
    Q_PROPERTY(bool available READ available NOTIFY menusChanged)
public:
    explicit AppMenu(QObject *parent = nullptr);
    QString serviceName() const { return m_service; }
    void setServiceName(const QString &s);
    QString objectPath() const { return m_path; }
    void setObjectPath(const QString &p);
    QVariantList menus() const { return m_menus; }
    bool available() const { return !m_menus.isEmpty(); }

    //! Items of top-level menu \a index: [{text, shortcut, enabled, checkable,
    //! checked, separator, path, children}]. `path` identifies the action for trigger().
    Q_INVOKABLE QVariantList items(int index) const;
    Q_INVOKABLE void trigger(const QString &path);

Q_SIGNALS:
    void sourceChanged();
    void menusChanged();

private:
    void rebuild();
    void reimport();
    QVariantList describe(QMenu *menu, const QString &prefix, int depth) const;
    QAction *actionAt(const QString &path) const;

    QString m_service, m_path;
    DBusMenuImporter *m_importer = nullptr;
    QVariantList m_menus;
};
}
