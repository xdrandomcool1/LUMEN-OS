// SPDX-License-Identifier: GPL-3.0-or-later
#include "document.h"
#include <QFile>
#include <QSaveFile>

namespace Lumen {
void Document::setText(const QString &t)
{
    m_text = t;
    m_lines = t.count(QLatin1Char('\n')) + 1;
    m_words = countWords(t);
    Q_EMIT statsChanged();
}

int Document::countWords(const QString &t)
{
    int n = 0;
    bool in = false;
    for (const QChar c : t) {
        const bool w = !c.isSpace();
        if (w && !in) ++n;
        in = w;
    }
    return n;
}

QVariantMap Document::load(const QUrl &url) const
{
    QFile f(url.isLocalFile() ? url.toLocalFile() : url.toString());
    if (!f.open(QIODevice::ReadOnly))
        return {{QStringLiteral("ok"), false}, {QStringLiteral("error"), f.errorString()}};
    if (f.size() > 64 * 1024 * 1024)
        return {{QStringLiteral("ok"), false}, {QStringLiteral("error"), tr("File is too large")}};
    return {{QStringLiteral("ok"), true}, {QStringLiteral("text"), QString::fromUtf8(f.readAll())}};
}

QString Document::save(const QUrl &url, const QString &text) const
{
    QSaveFile f(url.isLocalFile() ? url.toLocalFile() : url.toString());
    if (!f.open(QIODevice::WriteOnly)) return f.errorString();
    f.write(text.toUtf8());
    return f.commit() ? QString() : f.errorString();
}

int Document::count(const QString &h, const QString &n) const { return n.isEmpty() ? 0 : int(h.count(n)); }
}
