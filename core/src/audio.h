// SPDX-License-Identifier: GPL-3.0-or-later
// Output volume / device selection (PipeWire through pipewire-pulse).
#pragma once
#include <QObject>
#include <QPointer>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

namespace PulseAudioQt { class Sink; }

namespace Lumen {
class Audio : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool available READ available NOTIFY changed)
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY changed)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY changed)
    Q_PROPERTY(QVariantList outputs READ outputs NOTIFY outputsChanged)
    Q_PROPERTY(QString symbol READ symbol NOTIFY changed)
public:
    explicit Audio(QObject *parent = nullptr);
    bool available() const;
    qreal volume() const;
    void setVolume(qreal v);
    bool muted() const;
    void setMuted(bool m);
    QVariantList outputs() const;
    QString symbol() const;
    Q_INVOKABLE void setDefaultOutput(const QString &name);
Q_SIGNALS:
    void changed();
    void outputsChanged();
private:
    void trackDefault();
    QPointer<PulseAudioQt::Sink> m_sink;
};
}
