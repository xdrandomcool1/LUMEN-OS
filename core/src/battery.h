// SPDX-License-Identifier: GPL-3.0-or-later
// UPower display device (aggregate battery) for menu bar + Control Center.
#pragma once
#include <QObject>
#include <QtQml/qqmlregistration.h>

namespace Lumen {
class Battery : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool present READ present NOTIFY changed)
    Q_PROPERTY(qreal percentage READ percentage NOTIFY changed)
    Q_PROPERTY(bool charging READ charging NOTIFY changed)
    Q_PROPERTY(bool pluggedIn READ pluggedIn NOTIFY changed)
    Q_PROPERTY(qint64 timeToEmpty READ timeToEmpty NOTIFY changed)
    Q_PROPERTY(qint64 timeToFull READ timeToFull NOTIFY changed)
    Q_PROPERTY(QString symbol READ symbol NOTIFY changed)
public:
    explicit Battery(QObject *parent = nullptr);
    bool present() const { return m_present; }
    qreal percentage() const { return m_percentage; }
    bool charging() const { return m_state == 1 || m_state == 5; }
    bool pluggedIn() const { return m_onAc; }
    qint64 timeToEmpty() const { return m_tte; }
    qint64 timeToFull() const { return m_ttf; }
    QString symbol() const;
Q_SIGNALS:
    void changed();
private Q_SLOTS:
    void refresh();
private:
    bool m_present = false;
    qreal m_percentage = 0;
    uint m_state = 0;
    bool m_onAc = true;
    qint64 m_tte = 0, m_ttf = 0;
};
}
