// SPDX-License-Identifier: GPL-3.0-or-later
// LUMEN Liquid Glass composite.
//
// Layers (back to front): soft shadow → refracted, saturated, blurred backdrop
// → adaptive tint → specular rim + bevel light → (optional) contrast outline.
// The shape is a rounded rectangle with "continuous" (superelliptic) corners,
// evaluated as a signed distance field so every effect is resolution
// independent and anti-aliased.
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
    float hasBackdrop;   // 1: in-scene backdrop, 0: compositor provides blur
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
};
layout(binding = 1) uniform sampler2D backdrop;

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

vec3 sampleBackdrop(vec2 px)
{
    vec2 uv = backdropRect.xy + (px / itemSize) * backdropRect.zw;
    return texture(backdrop, uv).rgb;
}

float luma(vec3 c) { return dot(c, vec3(0.2126, 0.7152, 0.0722)); }

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
        float lw = max(min(lensWidth, r), 1.0);
        float t = clamp(1.0 - inside / lw, 0.0, 1.0);      // 1 at rim, 0 past bevel
        float bevel = pow(t, 1.7);                            // convex bevel profile

        float lit = dot(nrm, -lightDir);                    // >0 facing the light
        vec3 col;
        float alpha;
        if (hasBackdrop > 0.5) {
            // Refraction: sample outward along the normal; the bevel bends
            // the backdrop beyond the edge into the rim, like thick glass.
            vec2 disp = nrm * refraction * bevel;
            vec3 c;
            c.r = sampleBackdrop(px + disp * (1.0 + dispersion)).r;
            c.g = sampleBackdrop(px + disp).g;
            c.b = sampleBackdrop(px + disp * (1.0 - dispersion)).b;
            float L = luma(c);
            c = mix(vec3(L), c, saturation) * brightness;
            // Adaptive tint: the further the backdrop is from the tint's
            // luminance, the more tint is applied, keeping content legible.
            float tl = luma(tint.rgb);
            float adapt = adaptivity * smoothstep(0.12, 0.65, abs(L - tl));
            col = mix(c, tint.rgb, clamp(tint.a + adapt * (1.0 - tint.a) * 0.5, 0.0, 1.0));
            alpha = 1.0;
        } else {
            // Compositor-blur mode: the backdrop is blurred behind us by
            // the compositor; we only contribute tint and light.
            col = tint.rgb;
            alpha = clamp(tint.a + 0.12, 0.0, 1.0);
        }

        // Bevel shading: light side brightens, far side darkens subtly.
        float shade = bevel * (0.10 * max(lit, 0.0) - 0.06 * max(-lit, 0.0));
        col = clamp(col + vec3(shade) * (dark > 0.5 ? 0.6 : 1.0), 0.0, 1.0);

        // Specular rim: a crisp 1–1.5 px highlight concentrated where the
        // rim faces the light, plus a fainter counter-highlight opposite.
        // Inner glow: light scattered inside the bevel, strongest on the lit
        // edge — gives the surface its thickness.
        float glowBand = (1.0 - smoothstep(0.0, lw * 0.9, inside));
        float innerGlow = glowBand * glowBand * (0.10 + 0.14 * max(lit, 0.0) + 0.06 * max(-lit, 0.0)) * specular;
        col = 1.0 - (1.0 - col) * (1.0 - vec3(innerGlow));
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
