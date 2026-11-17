// SPDX-License-Identifier: GPL-3.0-or-later
#include "compositorglass.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QJsonDocument>
#include <QPointer>

namespace Lumen {

namespace {

const QString kService = QStringLiteral("org.kde.KWin");
const QString kPath = QStringLiteral("/org/lumen/Glass");
const QString kInterface = QStringLiteral("org.lumen.Glass");

//! Process-wide availability of the KWin effect, shared by all instances.
class Probe : public QObject
{
    Q_OBJECT
public:
    static Probe *instance()
    {
        static Probe *p = new Probe;
        return p;
    }
    bool available = false;

    void probe()
    {
        auto msg = QDBusMessage::createMethodCall(kService, kPath, kInterface, QStringLiteral("Available"));
        auto *watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(msg, 2000), this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *w) {
            QDBusPendingReply<bool> reply = *w;
            setAvailable(!reply.isError() && reply.value());
            w->deleteLater();
        });
    }
    void setAvailable(bool a)
    {
        if (available == a)
            return;
        available = a;
        Q_EMIT availableChanged();
    }

Q_SIGNALS:
    void availableChanged();
    //! The effect (re)loaded: every surface must publish its shapes again.
    void restarted();

private Q_SLOTS:
    void onStarted()
    {
        setAvailable(true);
        Q_EMIT restarted();
    }

private:
    Probe()
    {
        QDBusConnection::sessionBus().connect(QString(), kPath, kInterface, QStringLiteral("Started"), this, SLOT(onStarted()));
        probe();
    }
};

} // namespace

CompositorGlass::CompositorGlass(QObject *parent)
    : QObject(parent)
{
    m_timer.setSingleShot(true);
    m_timer.setInterval(0); // coalesce changes within one event-loop pass
    connect(&m_timer, &QTimer::timeout, this, &CompositorGlass::send);
    Probe *probe = Probe::instance();
    connect(probe, &Probe::availableChanged, this, [this] {
        Q_EMIT availableChanged();
        schedule();
    });
    connect(probe, &Probe::restarted, this, [this] {
        m_sentScope.clear();
        m_sentOutput.clear();
        schedule();
    });
}

CompositorGlass::~CompositorGlass()
{
    if (!m_sentScope.isEmpty())
        sendEmpty(m_sentScope, m_sentOutput);
}

bool CompositorGlass::available() const
{
    return Probe::instance()->available;
}

void CompositorGlass::setScope(const QString &s)
{
    if (m_scope == s)
        return;
    m_scope = s;
    Q_EMIT changed();
    schedule();
}

void CompositorGlass::setOutput(const QString &o)
{
    if (m_output == o)
        return;
    m_output = o;
    Q_EMIT changed();
    schedule();
}

void CompositorGlass::setShapes(const QVariantList &s)
{
    if (m_shapes == s)
        return;
    m_shapes = s;
    Q_EMIT changed();
    schedule();
}

void CompositorGlass::setEnabled(bool e)
{
    if (m_enabled == e)
        return;
    m_enabled = e;
    Q_EMIT changed();
    Q_EMIT availableChanged();
    schedule();
}

void CompositorGlass::schedule()
{
    m_timer.start();
}

void CompositorGlass::sendEmpty(const QString &scope, const QString &output)
{
    auto msg = QDBusMessage::createMethodCall(kService, kPath, kInterface, QStringLiteral("SetShapes"));
    msg << scope << output << QStringLiteral("[]");
    QDBusConnection::sessionBus().asyncCall(msg);
}

void CompositorGlass::send()
{
    // Clear the old key when the surface moved to another scope/output.
    if (!m_sentScope.isEmpty() && (m_sentScope != m_scope || m_sentOutput != m_output)) {
        sendEmpty(m_sentScope, m_sentOutput);
        m_sentScope.clear();
        m_sentOutput.clear();
    }
    if (!available() || m_scope.isEmpty())
        return;
    const bool publish = m_enabled && !m_shapes.isEmpty();
    if (!publish && m_sentScope.isEmpty())
        return;

    const QString json = publish ? QString::fromUtf8(QJsonDocument::fromVariant(m_shapes).toJson(QJsonDocument::Compact))
                                 : QStringLiteral("[]");
    auto msg = QDBusMessage::createMethodCall(kService, kPath, kInterface, QStringLiteral("SetShapes"));
    msg << m_scope << m_output << json;
    auto *watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(msg, 2000), this);
    QPointer<CompositorGlass> self(this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [self](QDBusPendingCallWatcher *w) {
        if (w->isError() && self) {
            // The effect went away (disabled, compositing suspended):
            // fall back until it announces itself again.
            Probe::instance()->setAvailable(false);
        }
        w->deleteLater();
    });
    if (publish) {
        m_sentScope = m_scope;
        m_sentOutput = m_output;
    } else {
        m_sentScope.clear();
        m_sentOutput.clear();
    }
}

} // namespace Lumen

#include "compositorglass.moc"
