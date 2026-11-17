// SPDX-License-Identifier: GPL-3.0-or-later
// System-level actions the shell can trigger on behalf of the user, plus the
// hardware facts they are gated on.
#pragma once

#include <QProcess>
#include <QQmlEngine>
#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

namespace Lumen {

/*!
 * The NVIDIA flow shells out to /usr/libexec/lumen-nvidia-setup via pkexec;
 * its stdout lines surface as \c status so the Control Center can show
 * progress. Everything is logged to the journal by the helper itself.
 */
class SystemActions : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool nvidiaHardware READ nvidiaHardware CONSTANT)
    //! NVIDIA hardware that is currently running on a software rasteriser.
    Q_PROPERTY(bool needsNvidiaDriver READ needsNvidiaDriver CONSTANT)
    Q_PROPERTY(bool installing READ installing NOTIFY installingChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)

public:
    static SystemActions *instance();
    static SystemActions *create(QQmlEngine *, QJSEngine *);

    bool nvidiaHardware() const { return m_nvidiaHardware; }
    bool needsNvidiaDriver() const;
    bool installing() const { return m_installing; }
    QString status() const { return m_status; }

    //! Adds the RPM Fusion repositories and installs akmod-nvidia; ask the
    //! user to reboot once it finishes (done()).
    Q_INVOKABLE void installNvidiaDriver();

Q_SIGNALS:
    void installingChanged();
    void statusChanged();
    void failed(const QString &message);
    void done();

private:
    explicit SystemActions(QObject *parent = nullptr);
    static bool detectNvidiaHardware();

    QProcess m_process;
    bool m_nvidiaHardware = false;
    bool m_installing = false;
    QString m_status;
};

} // namespace Lumen
