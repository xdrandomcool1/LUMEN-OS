// SPDX-License-Identifier: GPL-3.0-or-later
// Display brightness through PowerDevil (org.kde.ScreenBrightness), which
// handles backlight (via logind) and DDC/CI external monitors.
#pragma once
#include <QObject>
#include <QtQml/qqmlregistration.h>

namespace Lumen {
class Brightness : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool available READ available NOTIFY changed)
    Q_PROPERTY(qreal value READ value WRITE setValue NOTIFY changed)
    Q_PROPERTY(QString display READ display NOTIFY changed)
public:
    explicit Brightness(QObject *parent = nullptr);
    bool available() const { return !m_display.isEmpty() && m_max > 0; }
    qreal value() const { return m_max > 0 ? qreal(m_value) / m_max : 0; }
    void setValue(qreal v);
    QString display() const { return m_label; }
    Q_INVOKABLE void refresh();
Q_SIGNALS:
    void changed();
private Q_SLOTS:
    void onBrightnessChanged(const QString &display, int value);
private:
    QString m_display; // D-Bus name of the preferred (internal) display
    QString m_label;
    int m_value = 0;
    int m_max = 0;
};
}
