// SPDX-License-Identifier: GPL-3.0-or-later
// Liquid Glass material table. Header-only so it can be shared by the QML
// module (Lumen.Glass) and compositor-side renderers (KWin effect).
#pragma once

namespace Lumen::GlassMaterials {

enum Material {
    UltraThin = 0,
    Thin,
    Regular,
    Frosted,
    Dark,
    Light,
    Floating,
    Active,
    Count
};

struct Rgb { float r, g, b; };

struct Params {
    float blur;        // logical px; selects blur pyramid strength
    float saturation;  // vibrancy of backdrop
    float brightness;  // multiplier on backdrop
    Rgb tintLight;
    Rgb tintDark;
    float tintAlpha;   // base tint mix
    float adaptivity;  // how strongly tint fights backdrop luminance
    float refraction;  // max displacement at the rim (px)
    float lensWidth;   // width of refractive bevel (px)
    float dispersion;  // chromatic aberration amount
    float specular;    // rim highlight strength
    float shadow;      // shadow alpha
    float shadowRadius;
    bool useAccentTint;
};

inline constexpr Params kTable[Count] = {
    // blur  sat   bri   tintLight                 tintDark                  tα    adapt  refr  lens  disp  spec  shad  shR   accent
    { 18.f, 1.35f, 1.06f, {1.00f, 1.00f, 1.00f}, {0.11f, 0.11f, 0.12f}, 0.06f, 0.30f, 22.f, 22.f, 0.35f, 1.00f, 0.22f, 22.f, false }, // UltraThin
    { 24.f, 1.30f, 1.03f, {0.97f, 0.97f, 0.98f}, {0.13f, 0.13f, 0.14f}, 0.18f, 0.40f, 12.f, 16.f, 0.25f, 0.80f, 0.24f, 26.f, false }, // Thin
    { 30.f, 1.25f, 1.00f, {0.95f, 0.95f, 0.96f}, {0.12f, 0.12f, 0.13f}, 0.32f, 0.50f, 12.f, 18.f, 0.22f, 0.70f, 0.28f, 34.f, false }, // Regular
    { 44.f, 1.15f, 1.00f, {0.94f, 0.94f, 0.95f}, {0.10f, 0.10f, 0.11f}, 0.50f, 0.55f,  8.f, 16.f, 0.15f, 0.55f, 0.26f, 30.f, false }, // Frosted
    { 30.f, 1.30f, 0.90f, {0.06f, 0.06f, 0.07f}, {0.06f, 0.06f, 0.07f}, 0.50f, 0.45f, 12.f, 18.f, 0.20f, 0.60f, 0.30f, 34.f, false }, // Dark
    { 30.f, 1.30f, 1.06f, {1.00f, 1.00f, 1.00f}, {1.00f, 1.00f, 1.00f}, 0.50f, 0.45f, 12.f, 18.f, 0.20f, 0.75f, 0.20f, 30.f, false }, // Light
    { 22.f, 1.35f, 1.06f, {1.00f, 1.00f, 1.00f}, {0.16f, 0.16f, 0.17f}, 0.10f, 0.35f, 24.f, 24.f, 0.40f, 1.00f, 0.26f, 30.f, false }, // Floating
    { 22.f, 1.30f, 1.00f, {0.04f, 0.52f, 1.00f}, {0.04f, 0.52f, 1.00f}, 0.30f, 0.30f, 16.f, 18.f, 0.30f, 0.95f, 0.22f, 22.f, true  }, // Active
};

inline const Params &params(int m)
{
    return kTable[(m >= 0 && m < Count) ? m : Regular];
}

} // namespace Lumen::GlassMaterials
