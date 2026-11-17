// SPDX-License-Identifier: GPL-3.0-or-later
#include "glass.h"
#include "materials.h"

#include <QVector4D>

namespace Lumen {

QVariantMap Glass::material(int material, bool dark, const QColor &accent) const
{
    const auto &p = GlassMaterials::params(material);
    const auto &t = dark ? p.tintDark : p.tintLight;
    QVector4D tint(t.r, t.g, t.b, p.tintAlpha);
    if (p.useAccentTint && accent.isValid())
        tint = QVector4D(accent.redF(), accent.greenF(), accent.blueF(), p.tintAlpha);
    // Dark glass needs a little more tint to keep contrast on bright wallpapers.
    if (dark && material != Dark && material != Light)
        tint.setW(qMin(1.0f, tint.w() + 0.03f));
    return {
        {QStringLiteral("blur"), p.blur},
        {QStringLiteral("saturation"), p.saturation},
        {QStringLiteral("brightness"), dark ? p.brightness * 0.92f : p.brightness},
        {QStringLiteral("tint"), tint},
        {QStringLiteral("adaptivity"), p.adaptivity},
        {QStringLiteral("refraction"), p.refraction},
        {QStringLiteral("lensWidth"), p.lensWidth},
        {QStringLiteral("dispersion"), p.dispersion},
        {QStringLiteral("specular"), dark ? p.specular * 0.8f : p.specular},
        {QStringLiteral("shadow"), dark ? p.shadow * 1.4f : p.shadow},
        {QStringLiteral("shadowRadius"), p.shadowRadius},
    };
}

} // namespace Lumen
