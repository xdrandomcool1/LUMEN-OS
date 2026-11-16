// SPDX-License-Identifier: GPL-3.0-or-later
#include "symbol.h"

#include <QCache>
#include <QFile>
#include <QHash>
#include <QMutex>
#include <QPainter>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QSvgRenderer>

namespace Lumen {

namespace {
QString weightDir(Symbol::Weight w)
{
    switch (w) {
    case Symbol::Bold: return QStringLiteral("bold");
    case Symbol::Fill: return QStringLiteral("fill");
    case Symbol::Regular: break;
    }
    return QStringLiteral("regular");
}

QMutex s_mutex;
QHash<QString, QByteArray> s_svgCache;
QCache<QString, QImage> s_rasterCache(2048); // cost = 1 per entry
} // namespace

Symbol::Symbol(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setAntialiasing(true);
    setImplicitSize(16, 16);
}

QByteArray Symbol::svgData(const QString &name, Weight weight)
{
    const QString key = weightDir(weight) + QLatin1Char('/') + name;
    QMutexLocker lock(&s_mutex);
    auto it = s_svgCache.constFind(key);
    if (it != s_svgCache.constEnd())
        return *it;

    QByteArray data;
    QFile res(QStringLiteral(":/lumen/symbols/%1.svg").arg(key));
    if (res.open(QIODevice::ReadOnly)) {
        data = res.readAll();
    } else {
        const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                    QStringLiteral("lumen/symbols/%1.svg").arg(key));
        QFile f(path);
        if (!path.isEmpty() && f.open(QIODevice::ReadOnly))
            data = f.readAll();
        else if (weight != Regular) {
            lock.unlock();
            return svgData(name, Regular);
        }
    }
    s_svgCache.insert(key, data);
    return data;
}

void Symbol::setName(const QString &n)
{
    if (m_name == n)
        return;
    m_name = n;
    Q_EMIT nameChanged();
    update();
}

void Symbol::setColor(const QColor &c)
{
    if (m_color == c)
        return;
    m_color = c;
    Q_EMIT colorChanged();
    update();
}

void Symbol::setWeight(Weight w)
{
    if (m_weight == w)
        return;
    m_weight = w;
    Q_EMIT weightChanged();
    update();
}

bool Symbol::isValid() const
{
    return !m_name.isEmpty() && !svgData(m_name, m_weight).isEmpty();
}

void Symbol::paint(QPainter *painter)
{
    if (m_name.isEmpty())
        return;
    const qreal dpr = window() ? window()->effectiveDevicePixelRatio() : 1.0;
    const QSize px = (QSizeF(width(), height()) * dpr).toSize();
    if (px.isEmpty())
        return;

    const QString key = QStringLiteral("%1|%2|%3|%4x%5")
                            .arg(m_name).arg(int(m_weight)).arg(m_color.name(QColor::HexArgb))
                            .arg(px.width()).arg(px.height());
    QImage image;
    {
        QMutexLocker lock(&s_mutex);
        if (QImage *cached = s_rasterCache.object(key))
            image = *cached;
    }
    if (image.isNull()) {
        QByteArray svg = svgData(m_name, m_weight);
        if (svg.isEmpty())
            return;
        svg.replace("currentColor", m_color.name(QColor::HexRgb).toLatin1());
        QSvgRenderer renderer(svg);
        image = QImage(px, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        QPainter p(&image);
        p.setRenderHint(QPainter::Antialiasing);
        // Keep aspect ratio, centered.
        const QSizeF def = renderer.defaultSize();
        QSizeF target = def.scaled(QSizeF(px), Qt::KeepAspectRatio);
        renderer.render(&p, QRectF(QPointF((px.width() - target.width()) / 2.0,
                                           (px.height() - target.height()) / 2.0), target));
        p.end();
        if (m_color.alphaF() < 1.0) {
            QImage faded(px, QImage::Format_ARGB32_Premultiplied);
            faded.fill(Qt::transparent);
            QPainter fp(&faded);
            fp.setOpacity(m_color.alphaF());
            fp.drawImage(0, 0, image);
            fp.end();
            image = faded;
        }
        QMutexLocker lock(&s_mutex);
        s_rasterCache.insert(key, new QImage(image));
    }
    image.setDevicePixelRatio(dpr);
    painter->drawImage(QRectF(0, 0, width(), height()), image);
}

} // namespace Lumen
