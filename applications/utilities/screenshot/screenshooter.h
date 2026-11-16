// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QImage>
#include <QObject>
#include <QQuickImageProvider>
#include <QtQml/qqmlregistration.h>

namespace Lumen {
class Screenshooter : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString lastPath READ lastPath NOTIFY captured)
    Q_PROPERTY(QString error READ error NOTIFY failed)
public:
    explicit Screenshooter(QObject *parent = nullptr);
    QString lastPath() const { return m_lastPath; }
    QString error() const { return m_error; }
    //! mode: "screen" | "window" | "freeze" (for area selection)
    Q_INVOKABLE void capture(const QString &mode, int delayMs);
    //! Saves the frozen image cropped to \a rect (logical coordinates).
    Q_INVOKABLE void saveCrop(const QRectF &rect);
    Q_INVOKABLE void openLast();
    static QImage frozen();
Q_SIGNALS:
    void captured(const QString &path);
    void frozenReady();
    void failed();
private:
    void save(const QImage &img);
    QString m_lastPath, m_error;
};

class FrozenProvider : public QQuickImageProvider
{
public:
    FrozenProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}
    QImage requestImage(const QString &, QSize *size, const QSize &) override;
};
}
