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
    float refraction;  // glass thickness at the bevel (px); drives Snell refraction
    float lensWidth;   // width of refractive bevel (px)
    float dispersion;  // chromatic aberration amount
    float specular;    // rim highlight strength
    float shadow;      // shadow alpha
    float shadowRadius;
    bool useAccentTint;
    float frost;       // 0 = clear Liquid Glass (sharp, refracted backdrop) … 1 = frosted
    float ior;         // index of refraction of the bevel
};

inline constexpr Params kTable[Count] = {
    // refr = glass thickness (px) that drives the Snell refraction at the
    // bevel; lens = bevel width (px). Clear materials keep a sharp backdrop
    // and almost no tint: the look comes from lensing, not from blur.
    // blur  sat   bri   tintLight                 tintDark                  tα    adapt  refr  lens  disp  spec  shad  shR   accent frost  ior
    { 18.f, 1.20f, 1.04f, {1.00f, 1.00f, 1.00f}, {0.11f, 0.11f, 0.12f}, 0.02f, 0.12f, 22.f, 20.f, 0.60f, 1.00f, 0.22f, 22.f, false, 0.00f, 1.50f }, // UltraThin (clear)
    { 24.f, 1.25f, 1.03f, {0.97f, 0.97f, 0.98f}, {0.13f, 0.13f, 0.14f}, 0.08f, 0.30f, 16.f, 16.f, 0.45f, 0.90f, 0.24f, 26.f, false, 0.25f, 1.50f }, // Thin
    { 30.f, 1.25f, 1.00f, {0.95f, 0.95f, 0.96f}, {0.12f, 0.12f, 0.13f}, 0.12f, 0.28f, 16.f, 16.f, 0.35f, 0.80f, 0.28f, 34.f, false, 0.40f, 1.50f }, // Regular
    { 44.f, 1.15f, 1.00f, {0.94f, 0.94f, 0.95f}, {0.10f, 0.10f, 0.11f}, 0.40f, 0.55f, 10.f, 12.f, 0.20f, 0.60f, 0.26f, 30.f, false, 1.00f, 1.50f }, // Frosted
    { 30.f, 1.30f, 0.90f, {0.06f, 0.06f, 0.07f}, {0.06f, 0.06f, 0.07f}, 0.42f, 0.45f, 14.f, 14.f, 0.30f, 0.65f, 0.30f, 34.f, false, 0.60f, 1.50f }, // Dark
    { 30.f, 1.30f, 1.06f, {1.00f, 1.00f, 1.00f}, {1.00f, 1.00f, 1.00f}, 0.38f, 0.45f, 14.f, 14.f, 0.30f, 0.80f, 0.20f, 30.f, false, 0.60f, 1.50f }, // Light
    { 22.f, 1.25f, 1.05f, {1.00f, 1.00f, 1.00f}, {0.16f, 0.16f, 0.17f}, 0.03f, 0.18f, 24.f, 24.f, 0.65f, 1.00f, 0.26f, 30.f, false, 0.05f, 1.52f }, // Floating
    { 22.f, 1.30f, 1.00f, {0.04f, 0.52f, 1.00f}, {0.04f, 0.52f, 1.00f}, 0.30f, 0.30f, 14.f, 14.f, 0.40f, 0.95f, 0.22f, 22.f, true,  0.30f, 1.50f }, // Active
};

inline const Params &params(int m)
{
    return kTable[(m >= 0 && m < Count) ? m : Regular];
}

} // namespace Lumen::GlassMaterials
