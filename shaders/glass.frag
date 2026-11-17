// SPDX-License-Identifier: GPL-3.0-or-later
// LUMEN Liquid Glass composite.
//
// The glass is modelled as a thin slab lying on the backdrop whose edge is a
// convex bevel. Inside the bevel the surface height follows a "convex
// squircle" profile h(x) = T·(1 − (1 − x)⁴)^¼ (x = distance from the rim
// across the bevel, 0…1), which keeps the transition into the flat centre
// smooth. Each pixel shoots an orthogonal view ray into the surface, bends it
// with Snell's law (refract(), n = 1 → IOR) and follows it through the glass
// to the backdrop plane; the hit point is what we sample. That makes the rim
// magnify and bend the content behind it while the flat centre stays clear —
// the defining look of Liquid Glass, as opposed to frosted glass, which only
// blurs. Red/green/blue use slightly different IORs (dispersion).
//
// Layers (back to front): soft shadow → refracted backdrop (sharp, or mixed
// toward the blurred pyramid by `frost`) → adaptive tint → Fresnel
// reflection + specular rim → (optional) contrast outline. The shape is a
// rounded rectangle with "continuous" (superelliptic) corners, evaluated as a
// signed distance field so every effect is resolution independent and
// anti-aliased.
//
// References: Snell–Descartes refraction with a convex-squircle bezel as
// described in "Liquid Glass in the Browser: Refraction with CSS and SVG"
// (kube.io); SDF shapes after Inigo Quilez.
#version 440
layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec2 itemSize;       // size of the shader item (logical px)
    vec4 shape;          // glass rect inside the item: x, y, w, h
    float radius;        // corner radius
    float smoothness;    // corner exponent: 2 = circular, ~2.6 = continuous
    vec4 backdropRect;   // item rect in backdrop UV space (u, v, du, dv)
    float hasBackdrop;   // 1: in-scene backdrop, 0: none
    float compositorBlur; // 1: compositor blurs behind, 2: compositor refracts (lumen_glass)
    float refraction;    // max rim displacement (px)
    float lensWidth;     // width of the refracting bevel (px)
    float dispersion;    // chromatic dispersion (0..1)
    vec4 tint;           // rgb + mix alpha
    float adaptivity;    // luminance adaptation strength
    float saturation;
    float brightness;
    float specular;
    vec2 lightDir;       // direction light travels (screen space, normalized)
    vec4 shadowParams;   // alpha, blur radius, y offset, spread
    float outline;       // increase-contrast outline alpha
    float dark;          // 1 in dark appearance
    float pressed;       // 0..1 interactive "press" glow
    float frost;         // 0: clear (sharp backdrop) … 1: fully frosted (blurred)
    float ior;           // index of refraction of the glass (≈1.5)
};
layout(binding = 1) uniform sampler2D backdrop;       // blurred pyramid output
layout(binding = 2) uniform sampler2D sharpBackdrop;  // unblurred backdrop

float cornerNorm(vec2 q, float n)
{
    q = max(q, vec2(0.0));
    if (n <= 2.001)
        return length(q);
    return pow(pow(q.x, n) + pow(q.y, n), 1.0 / n);
}

// Signed distance to the rounded rectangle centred at the origin.
float sdShape(vec2 p, vec2 halfSize, float r, float n)
{
    vec2 q = abs(p) - halfSize + vec2(r);
    return cornerNorm(q, n) + min(max(q.x, q.y), 0.0) - r;
}

// Outward surface normal of the shape at p (analytic, continuous along rim).
vec2 shapeNormal(vec2 p, vec2 halfSize, float r, float n)
{
    vec2 q = abs(p) - halfSize + vec2(r);
    vec2 s = vec2(p.x < 0.0 ? -1.0 : 1.0, p.y < 0.0 ? -1.0 : 1.0);
    if (q.x > 0.0 && q.y > 0.0)
        return normalize(s * pow(q, vec2(n - 1.0)));
    return q.x > q.y ? vec2(s.x, 0.0) : vec2(0.0, s.y);
}

// Backdrop at item pixel px; `f` blends sharp (0) → blurred (1).
vec3 sampleBackdrop(vec2 px, float f)
{
    vec2 uv = backdropRect.xy + (px / itemSize) * backdropRect.zw;
    vec3 s = texture(sharpBackdrop, uv).rgb;
    vec3 b = texture(backdrop, uv).rgb;
    return mix(s, b, f);
}

float luma(vec3 c) { return dot(c, vec3(0.2126, 0.7152, 0.0722)); }

// Convex squircle bezel profile (unit height) and its slope, x in 0…1.
float bezelHeight(float x)
{
    float u = 1.0 - clamp(x, 0.0, 1.0);
    return pow(max(1.0 - u * u * u * u, 0.0), 0.25);
}
float bezelSlope(float x)
{
    // d/dx (1 − u⁴)^¼ = u³ (1 − u⁴)^−¾ ; clamp keeps the rim finite.
    float u = 1.0 - clamp(x, 0.004, 1.0);
    float k = max(1.0 - u * u * u * u, 1e-4);
    return u * u * u * pow(k, -0.75);
}

// Lateral offset (px) of a view ray refracted at a surface with normal N
// that then travels `depth` px through the glass to the backdrop plane.
vec2 refractOffset(vec3 N, float eta, float depth)
{
    vec3 T = refract(vec3(0.0, 0.0, -1.0), N, eta);
    if (dot(T, T) < 1e-6) // total internal reflection cannot happen entering glass; guard anyway
        return vec2(0.0);
    return T.xy * (depth / max(-T.z, 0.2));
}

void main()
{
    vec2 px = qt_TexCoord0 * itemSize;
    vec2 halfSize = shape.zw * 0.5;
    vec2 center = shape.xy + halfSize;
    vec2 p = px - center;
    float r = min(radius, min(halfSize.x, halfSize.y));
    float n = smoothness;

    float d = sdShape(p, halfSize, r, n);
    float aa = max(fwidth(d), 1e-3);
    float coverage = clamp(0.5 - d / aa, 0.0, 1.0);

    // ---- Shadow (outside the shape) -------------------------------------
    vec4 shadowColor = vec4(0.0);
    if (shadowParams.x > 0.0 && coverage < 1.0) {
        vec2 sp = p - vec2(0.0, shadowParams.z);
        float spread = shadowParams.w;
        float ds = sdShape(sp, halfSize + vec2(spread), r + spread, n);
        float sigma = max(shadowParams.y * 0.42, 0.5);
        float a = shadowParams.x * exp(-(max(ds, 0.0) * max(ds, 0.0)) / (2.0 * sigma * sigma));
        // Contact shadow: tighter, darker band hugging the edge.
        float contact = 0.35 * shadowParams.x * exp(-max(d, 0.0) / 1.6);
        a = clamp(a + contact, 0.0, 1.0);
        shadowColor = vec4(0.0, 0.0, 0.0, a);
    }

    vec4 glassColor = vec4(0.0);
    if (coverage > 0.0) {
        vec2 nrm = shapeNormal(p, halfSize, r, n);
        float inside = max(-d, 0.0);
        // Keep a flat, undistorted centre even on small shapes.
        float lw = max(min(min(lensWidth, r), 0.6 * min(halfSize.x, halfSize.y)), 1.0);
        float t = clamp(1.0 - inside / lw, 0.0, 1.0);      // 1 at rim, 0 past bevel
        float bevel = pow(t, 1.7);                            // convex bevel profile

        float lit = dot(nrm, -lightDir);                    // >0 facing the light
        vec3 col;
        float alpha;
        // 3D surface normal of the bevel: the surface rises inward, so the
        // normal tilts outward by the profile's slope (thickness / bezel).
        float x = clamp(inside / lw, 0.0, 1.0);
        float thick = max(refraction, 0.0);
        float slope = thick / lw * bezelSlope(x);
        vec3 N = normalize(vec3(nrm * slope, 1.0));
        float fresnelCos = N.z; // cos of the angle between view ray and normal
        if (hasBackdrop > 0.5) {
            // Refraction (Snell): follow the bent view ray through the glass
            // to the backdrop. The convex bevel bends rays inward, so the rim
            // magnifies and curves the content behind it; the flat centre
            // (slope 0) passes the backdrop through undistorted.
            float depth = thick * bezelHeight(x) + 0.35 * thick;
            float etaG = 1.0 / max(ior, 1.0001);
            float dIor = 0.045 * dispersion;
            vec2 offG = refractOffset(N, etaG, depth);
            vec2 offR = refractOffset(N, 1.0 / (ior - dIor), depth);
            vec2 offB = refractOffset(N, 1.0 / (ior + dIor), depth);
            // Exaggerate the per-channel split a little: physically correct
            // dispersion at these IORs is sub-pixel.
            offR = offG + (offR - offG) * 6.0;
            offB = offG + (offB - offG) * 6.0;
            // The bevel scatters a little light: soften it slightly more
            // than the centre so the rim reads as thick glass, not a decal.
            float f = clamp(frost + 0.3 * pow(1.0 - x, 2.0) * (1.0 - frost), 0.0, 1.0);
            vec3 c;
            c.r = sampleBackdrop(px + offR, f).r;
            c.g = sampleBackdrop(px + offG, f).g;
            c.b = sampleBackdrop(px + offB, f).b;
            float L = luma(c);
            c = mix(vec3(L), c, saturation) * brightness;
            // Adaptive tint: the further the backdrop is from the tint's
            // luminance, the more tint is applied, keeping content legible.
            float tl = luma(tint.rgb);
            float adapt = adaptivity * smoothstep(0.12, 0.65, abs(L - tl));
            col = mix(c, tint.rgb, clamp(tint.a + adapt * (1.0 - tint.a) * 0.5, 0.0, 1.0));
            alpha = 1.0;
        } else if (compositorBlur > 1.5) {
            // Compositor glass: KWin's lumen_glass effect has already drawn
            // the refracted backdrop under us; contribute tint and light
            // only. The backdrop is unknown here, so adaptivity becomes a
            // small constant veil that keeps content legible.
            col = tint.rgb;
            alpha = clamp(tint.a + 0.25 * adaptivity, 0.0, 1.0);
        } else {
            // Compositor-blur mode: the backdrop is blurred behind us by
            // the compositor; we only contribute tint and light.
            col = tint.rgb;
            alpha = compositorBlur > 0.5 ? clamp(tint.a + 0.18, 0.0, 1.0)
                                         : clamp(max(tint.a, 0.86), 0.0, 1.0);
        }

        // Bevel shading: light side brightens, far side darkens subtly.
        float shade = bevel * (0.06 * max(lit, 0.0) - 0.05 * max(-lit, 0.0));
        col = clamp(col + vec3(shade) * (dark > 0.5 ? 0.6 : 1.0), 0.0, 1.0);

        // Fresnel (Schlick, F0 = 0.04 for n ≈ 1.5): the steep bevel reflects
        // a soft, bright environment, strongest on the side facing the
        // light. The flat centre (normal facing the viewer) reflects ~4 %,
        // so it stays clear.
        float fres = 0.04 + 0.96 * pow(1.0 - clamp(fresnelCos, 0.0, 1.0), 5.0);
        vec3 env = dark > 0.5 ? vec3(0.78, 0.80, 0.86) : vec3(1.0);
        float envAmt = clamp(fres * specular * (0.45 + 0.35 * max(lit, 0.0)), 0.0, 0.85);
        if (hasBackdrop > 0.5) {
            col = mix(col, env, envAmt);
        } else if (compositorBlur > 1.5) {
            // Composite the reflection over the translucent tint (premultiplied).
            vec4 pm = vec4(col * alpha, alpha) * (1.0 - envAmt) + vec4(env * envAmt, envAmt);
            alpha = pm.a;
            col = alpha > 1e-4 ? pm.rgb / alpha : env;
        } else {
            col = mix(col, env, envAmt);
        }

        // Specular rim: a crisp 1–1.5 px highlight concentrated where the
        // rim faces the light, plus a fainter counter-highlight opposite.
        float rimW = 1.6;
        float rim = 1.0 - smoothstep(0.0, rimW, inside);
        float spec = rim * (0.22 + 0.78 * pow(max(lit, 0.0), 1.6));
        spec += rim * 0.45 * pow(max(-lit, 0.0), 2.0);
        // Broad glint across the bevel.
        spec += 0.18 * pow(bevel, 2.0) * pow(max(lit, 0.0), 3.0);
        spec *= specular;
        float specA = clamp(spec, 0.0, 1.0);
        // Screen blend keeps highlights luminous over any color.
        col = 1.0 - (1.0 - col) * (1.0 - vec3(specA));
        alpha = max(alpha, specA);

        // Inner edge glow: a wide, faint band of light just inside the rim
        // (macOS-27 "defined" glass) — brightest on the side facing the
        // light, scaled by the material's specular strength.
        float band = 1.0 - smoothstep(0.0, 14.0, inside);
        band *= band;
        float innerGlow = clamp(band * (0.05 + 0.10 * pow(max(lit, 0.0), 1.4)) * specular, 0.0, 1.0);
        col = 1.0 - (1.0 - col) * (1.0 - vec3(innerGlow));
        alpha = max(alpha, innerGlow);

        // Interactive press: soft inner light that blooms from the center.
        if (pressed > 0.0) {
            float glow = pressed * 0.18 * (1.0 - smoothstep(0.0, max(halfSize.x, halfSize.y), length(p)));
            col = 1.0 - (1.0 - col) * (1.0 - glow);
            alpha = max(alpha, glow);
        }

        // Increase-contrast outline.
        if (outline > 0.0) {
            float ol = 1.0 - smoothstep(0.0, 1.2, inside);
            vec3 oc = dark > 0.5 ? vec3(1.0) : vec3(0.0);
            col = mix(col, oc, ol * outline);
            alpha = max(alpha, ol * outline);
        }

        glassColor = vec4(col * alpha, alpha);
    }

    fragColor = (glassColor * coverage + shadowColor * (1.0 - coverage)) * qt_Opacity;
}
