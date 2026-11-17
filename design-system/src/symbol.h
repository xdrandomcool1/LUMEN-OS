// SPDX-License-Identifier: GPL-3.0-or-later
// Lumen.Design — tinted vector glyph.
#pragma once

#include <QColor>
#include <QImage>
#include <QQuickPaintedItem>
#include <QtQml/qqmlregistration.h>

namespace Lumen {

/*!
 * Renders a symbol SVG (fill="currentColor") in the requested color.
 * Symbols are looked up in the compiled-in set first (:/lumen/symbols/<weight>/)
 * and then in $XDG_DATA_DIRS/lumen/symbols/<weight>/. Rendered rasters are
 * cached process-wide per (name, weight, color, pixel size).
 */
class Symbol : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(Weight weight READ weight WRITE setWeight NOTIFY weightChanged)
    Q_PROPERTY(bool valid READ isValid NOTIFY nameChanged)

public:
    enum Weight { Regular, Bold, Fill };
    Q_ENUM(Weight)

    explicit Symbol(QQuickItem *parent = nullptr);

    QString name() const { return m_name; }
    void setName(const QString &n);
    QColor color() const { return m_color; }
    void setColor(const QColor &c);
    Weight weight() const { return m_weight; }
    void setWeight(Weight w);
    bool isValid() const;

    void paint(QPainter *painter) override;

    static QByteArray svgData(const QString &name, Weight weight);

Q_SIGNALS:
    void nameChanged();
    void colorChanged();
    void weightChanged();

private:
    QString m_name;
    QColor m_color = Qt::black;
    Weight m_weight = Regular;
};

} // namespace Lumen
