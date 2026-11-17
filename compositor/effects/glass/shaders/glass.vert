// SPDX-License-Identifier: GPL-3.0-or-later
#version 140

uniform mat4 modelViewProjectionMatrix;

in vec2 position;
in vec2 texcoord;

out vec2 pos; // device px relative to the captured backdrop (y down)

void main(void)
{
    gl_Position = modelViewProjectionMatrix * vec4(position, 0.0, 1.0);
    pos = texcoord;
}
