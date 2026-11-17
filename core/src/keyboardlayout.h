// SPDX-License-Identifier: GPL-3.0-or-later
// Keyboard layouts as configured in KWin (org.kde.keyboard).
#pragma once
#include <QObject>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

namespace Lumen {
class KeyboardLayout : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QVariantList layouts READ layouts NOTIFY changed)
    Q_PROPERTY(int current READ current WRITE setCurrent NOTIFY changed)
    Q_PROPERTY(QString shortName READ shortName NOTIFY changed)
public:
    explicit KeyboardLayout(QObject *parent = nullptr);
    QVariantList layouts() const { return m_layouts; }
    int current() const { return m_current; }
    void setCurrent(int i);
    QString shortName() const;
    Q_INVOKABLE void next();
Q_SIGNALS:
    void changed();
private Q_SLOTS:
    void refresh();
    void onLayoutChanged(uint index);
private:
    QVariantList m_layouts;
    int m_current = 0;
};
}
