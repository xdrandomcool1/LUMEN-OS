// SPDX-License-Identifier: GPL-3.0-or-later
#include "audio.h"
#include <PulseAudioQt/Context>
#include <PulseAudioQt/Server>
#include <PulseAudioQt/Sink>

using namespace PulseAudioQt;

namespace Lumen {
Audio::Audio(QObject *parent) : QObject(parent)
{
    auto *ctx = Context::instance();
    connect(ctx->server(), &Server::defaultSinkChanged, this, &Audio::trackDefault);
    connect(ctx, &Context::sinkAdded, this, [this] { trackDefault(); Q_EMIT outputsChanged(); });
    connect(ctx, &Context::sinkRemoved, this, [this] { trackDefault(); Q_EMIT outputsChanged(); });
    trackDefault();
}

void Audio::trackDefault()
{
    if (m_sink)
        disconnect(m_sink, nullptr, this, nullptr);
    m_sink = Context::instance()->server()->defaultSink();
    if (m_sink) {
        connect(m_sink, &Sink::volumeChanged, this, &Audio::changed);
        connect(m_sink, &Sink::mutedChanged, this, &Audio::changed);
    }
    Q_EMIT changed();
    Q_EMIT outputsChanged();
}

bool Audio::available() const { return m_sink != nullptr; }
qreal Audio::volume() const { return m_sink ? qreal(m_sink->volume()) / PulseAudioQt::normalVolume() : 0; }
void Audio::setVolume(qreal v)
{
    if (!m_sink) return;
    m_sink->setVolume(qint64(qBound<qreal>(0, v, 1) * PulseAudioQt::normalVolume()));
    if (v > 0 && m_sink->isMuted()) m_sink->setMuted(false);
}
bool Audio::muted() const { return m_sink && m_sink->isMuted(); }
void Audio::setMuted(bool m) { if (m_sink) m_sink->setMuted(m); }

QVariantList Audio::outputs() const
{
    QVariantList out;
    const auto def = Context::instance()->server()->defaultSink();
    for (Sink *s : Context::instance()->sinks())
        out.append(QVariantMap{{QStringLiteral("name"), s->name()}, {QStringLiteral("description"), s->description()},
                               {QStringLiteral("active"), s == def}});
    return out;
}

void Audio::setDefaultOutput(const QString &name)
{
    for (Sink *s : Context::instance()->sinks())
        if (s->name() == name) { s->setDefault(true); return; }
}

QString Audio::symbol() const
{
    const qreal v = volume();
    if (!m_sink || muted() || v <= 0.001) return QStringLiteral("speaker-simple-slash");
    if (v < 0.34) return QStringLiteral("speaker-simple-none");
    if (v < 0.67) return QStringLiteral("speaker-simple-low");
    return QStringLiteral("speaker-simple-high");
}
}
