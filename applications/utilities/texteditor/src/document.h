// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QObject>
#include <QUrl>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

namespace Lumen {
//! File I/O and statistics for the text editor (UTF-8, atomic saves).
class Document : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY statsChanged)
    Q_PROPERTY(int lines READ lines NOTIFY statsChanged)
    Q_PROPERTY(int words READ words NOTIFY statsChanged)
public:
    using QObject::QObject;
    QString text() const { return m_text; }
    void setText(const QString &t);
    int lines() const { return m_lines; }
    int words() const { return m_words; }
    Q_INVOKABLE QVariantMap load(const QUrl &url) const;
    Q_INVOKABLE QString save(const QUrl &url, const QString &text) const;
    Q_INVOKABLE int count(const QString &haystack, const QString &needle) const;
    static int countWords(const QString &t);
Q_SIGNALS:
    void statsChanged();
private:
    QString m_text;
    int m_lines = 1, m_words = 0;
};
}
