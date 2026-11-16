// SPDX-License-Identifier: GPL-3.0-or-later
// Lumen.Glass — material registry exposed to QML.
#pragma once

#include <QColor>
#include <QObject>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

namespace Lumen {

class Glass : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    enum Material { UltraThin, Thin, Regular, Frosted, Dark, Light, Floating, Active };
    Q_ENUM(Material)

    explicit Glass(QObject *parent = nullptr) : QObject(parent) {}

    //! Resolved shader parameters for \a material in the given theme.
    //! \a accent is used by materials with an accent tint (Active).
    Q_INVOKABLE QVariantMap material(int material, bool dark, const QColor &accent = QColor()) const;
};

} // namespace Lumen
