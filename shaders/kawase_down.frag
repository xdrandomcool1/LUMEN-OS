// SPDX-License-Identifier: GPL-3.0-or-later
// Dual-Kawase downsample pass (Marius Bjørge, "Bandwidth-efficient rendering").
#version 440
layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec2 texel;   // 1 / source texture size
    float offset; // blur spread in source texels
};
layout(binding = 1) uniform sampler2D source;

void main()
{
    vec2 uv = qt_TexCoord0;
    vec2 h = texel * 0.5 * offset;
    vec4 sum = texture(source, uv) * 4.0;
    sum += texture(source, uv - h);
    sum += texture(source, uv + h);
    sum += texture(source, uv + vec2(h.x, -h.y));
    sum += texture(source, uv - vec2(h.x, -h.y));
    fragColor = (sum / 8.0) * qt_Opacity;
}
