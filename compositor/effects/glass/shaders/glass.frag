// SPDX-License-Identifier: GPL-3.0-or-later
// Compositor-side Liquid Glass backdrop: the same refraction model as
// shaders/glass.frag in Lumen.Glass (convex squircle bevel, Snell's law,
// per-channel IOR), applied to the pixels KWin composited behind the shell
// surface. Tint, Fresnel and specular light are drawn by the shell itself.
#version 140

uniform sampler2D texUnit;  // captured backdrop (GL orientation, mipmapped when frosted)
uniform vec2 texSize;       // device px
uniform vec4 shape;         // x, y, w, h in device px, relative to the backdrop
uniform float radius;
uniform float smoothness;
uniform float thickness;
uniform float lensWidth;
uniform float dispersion;
uniform float ior;
uniform float frost;
uniform float saturation;
uniform float brightness;
uniform float opacity;

in vec2 pos;
out vec4 fragColor;

float cornerNorm(vec2 q, float n)
{
    q = max(q, vec2(0.0));
    if (n <= 2.001)
        return length(q);
    return pow(pow(q.x, n) + pow(q.y, n), 1.0 / n);
}

float sdShape(vec2 p, vec2 halfSize, float r, float n)
{
    vec2 q = abs(p) - halfSize + vec2(r);
    return cornerNorm(q, n) + min(max(q.x, q.y), 0.0) - r;
}

vec2 shapeNormal(vec2 p, vec2 halfSize, float r, float n)
{
    vec2 q = abs(p) - halfSize + vec2(r);
    vec2 s = vec2(p.x < 0.0 ? -1.0 : 1.0, p.y < 0.0 ? -1.0 : 1.0);
    if (q.x > 0.0 && q.y > 0.0)
        return normalize(s * pow(q, vec2(n - 1.0)));
    return q.x > q.y ? vec2(s.x, 0.0) : vec2(0.0, s.y);
}

float bezelHeight(float x)
{
    float u = 1.0 - clamp(x, 0.0, 1.0);
    return pow(max(1.0 - u * u * u * u, 0.0), 0.25);
}

float bezelSlope(float x)
{
    float u = 1.0 - clamp(x, 0.004, 1.0);
    float k = max(1.0 - u * u * u * u, 1e-4);
    return u * u * u * pow(k, -0.75);
}

vec2 refractOffset(vec3 N, float eta, float depth)
{
    vec3 T = refract(vec3(0.0, 0.0, -1.0), N, eta);
    if (dot(T, T) < 1e-6)
        return vec2(0.0);
    return T.xy * (depth / max(-T.z, 0.2));
}

vec3 sampleAt(vec2 px, float f)
{
    px = clamp(px, vec2(0.5), texSize - vec2(0.5));
    vec2 uv = vec2(px.x / texSize.x, 1.0 - px.y / texSize.y);
    if (f <= 0.001)
        return textureLod(texUnit, uv, 0.0).rgb;
    // Frost: sample the mip chain, with a small rotated kernel to hide the
    // box-filter blockiness of the higher levels.
    float lod = f * 5.0;
    vec2 o = exp2(lod) * 0.6 / texSize;
    vec3 c = textureLod(texUnit, uv, lod).rgb * 2.0;
    c += textureLod(texUnit, uv + vec2(o.x, o.y * 0.5), lod).rgb;
    c += textureLod(texUnit, uv + vec2(-o.x * 0.5, o.y), lod).rgb;
    c += textureLod(texUnit, uv + vec2(-o.x, -o.y * 0.5), lod).rgb;
    c += textureLod(texUnit, uv + vec2(o.x * 0.5, -o.y), lod).rgb;
    return c / 6.0;
}

void main(void)
{
    vec2 halfSize = shape.zw * 0.5;
    vec2 p = pos - (shape.xy + halfSize);
    float r = min(radius, min(halfSize.x, halfSize.y));
    float d = sdShape(p, halfSize, r, smoothness);
    float aa = max(fwidth(d), 1e-3);
    float coverage = clamp(0.5 - d / aa, 0.0, 1.0);
    if (coverage <= 0.0)
        discard;

    vec2 nrm = shapeNormal(p, halfSize, r, smoothness);
    float inside = max(-d, 0.0);
    float lw = max(min(min(lensWidth, r), 0.6 * min(halfSize.x, halfSize.y)), 1.0);
    float x = clamp(inside / lw, 0.0, 1.0);
    float thick = max(thickness, 0.0);
    float slope = thick / lw * bezelSlope(x);
    vec3 N = normalize(vec3(nrm * slope, 1.0));

    float depth = thick * bezelHeight(x) + 0.35 * thick;
    float dIor = 0.045 * dispersion;
    vec2 offG = refractOffset(N, 1.0 / max(ior, 1.0001), depth);
    vec2 offR = refractOffset(N, 1.0 / (ior - dIor), depth);
    vec2 offB = refractOffset(N, 1.0 / (ior + dIor), depth);
    offR = offG + (offR - offG) * 6.0;
    offB = offG + (offB - offG) * 6.0;
    float f = clamp(frost + 0.3 * pow(1.0 - x, 2.0) * (1.0 - frost), 0.0, 1.0);

    vec3 c;
    c.r = sampleAt(pos + offR, f).r;
    c.g = sampleAt(pos + offG, f).g;
    c.b = sampleAt(pos + offB, f).b;
    float L = dot(c, vec3(0.2126, 0.7152, 0.0722));
    c = clamp(mix(vec3(L), c, saturation) * brightness, 0.0, 1.0);

    float a = coverage * opacity;
    fragColor = vec4(c * a, a);
}
