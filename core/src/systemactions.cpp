// SPDX-License-Identifier: GPL-3.0-or-later
#include "systemactions.h"
#include "gpu.h"

#include <QDir>
#include <QFile>
#include <QQmlEngine>

namespace Lumen {

bool SystemActions::detectNvidiaHardware()
{
    const QDir devices(QStringLiteral("/sys/class/drm"));
    const auto cards = devices.entryList(QStringList(QStringLiteral("card*")),
                                         QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &card : cards) {
        QFile f(devices.filePath(card + QStringLiteral("/device/vendor")));
        if (!f.open(QIODevice::ReadOnly))
            continue;
        if (QString::fromLatin1(f.readAll().trimmed()) == QStringLiteral("0x10de"))
            return true;
    }
    return false;
}

SystemActions::SystemActions(QObject *parent)
    : QObject(parent)
    , m_nvidiaHardware(detectNvidiaHardware())
{
    // Surface the helper's last progress line; keep the previous status when
    // a chunk contains only the tail of a line.
    connect(&m_process, &QProcess::readyReadStandardOutput, this, [this] {
        const QStringList lines = QString::fromUtf8(m_process.readAllStandardOutput())
                                      .split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        for (const QString &line : lines) {
            const QString trimmed = line.trimmed();
            if (!trimmed.isEmpty())
                m_status = trimmed;
        }
        Q_EMIT statusChanged();
    });
    connect(&m_process, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus) {
        m_installing = false;
        Q_EMIT installingChanged();
        if (exitCode == 0) {
            m_status = QStringLiteral("Reboot to activate the NVIDIA driver.");
            Q_EMIT statusChanged();
            Q_EMIT done();
        } else {
            const QString err = QString::fromUtf8(m_process.readAllStandardError()).simplified();
            m_status = err.left(160);
            if (m_status.isEmpty())
                m_status = QStringLiteral("The driver setup failed (exit %1).").arg(exitCode);
            Q_EMIT statusChanged();
            Q_EMIT failed(m_status);
        }
    });
    connect(&m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError) {
        if (!m_installing)
            return;
        m_installing = false;
        Q_EMIT installingChanged();
        m_status = QStringLiteral("Could not start the driver setup (pkexec).");
        Q_EMIT statusChanged();
        Q_EMIT failed(m_status);
    });
}

SystemActions *SystemActions::instance()
{
    static SystemActions *s = new SystemActions;
    return s;
}

SystemActions *SystemActions::create(QQmlEngine *, QJSEngine *)
{
    QQmlEngine::setObjectOwnership(instance(), QQmlEngine::CppOwnership);
    return instance();
}

bool SystemActions::needsNvidiaDriver() const
{
    return m_nvidiaHardware && Gpu::instance()->software();
}

void SystemActions::installNvidiaDriver()
{
    if (m_installing)
        return;
    m_installing = true;
    m_status = QStringLiteral("Starting the NVIDIA driver setup…");
    Q_EMIT installingChanged();
    Q_EMIT statusChanged();
    m_process.start(QStringLiteral("pkexec"),
                    {QStringLiteral(LUMEN_LIBEXECDIR "/lumen-nvidia-setup")});
}

} // namespace Lumen
