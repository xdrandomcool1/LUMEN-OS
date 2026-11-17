// SPDX-License-Identifier: GPL-3.0-or-later
// Clips a layer to a continuous-corner rounded rectangle (anti-aliased).
#version 440
layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec2 itemSize;
    float radius;
};
layout(binding = 1) uniform sampler2D source;

void main()
{
    vec2 p = qt_TexCoord0 * itemSize - itemSize * 0.5;
    vec2 q = abs(p) - itemSize * 0.5 + vec2(radius);
    vec2 m = max(q, vec2(0.0));
    float n = 2.6;
    float d = pow(pow(m.x, n) + pow(m.y, n), 1.0 / n) + min(max(q.x, q.y), 0.0) - radius;
    float cov = clamp(0.5 - d / max(fwidth(d), 1e-3), 0.0, 1.0);
    fragColor = texture(source, qt_TexCoord0) * cov * qt_Opacity;
}
