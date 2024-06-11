#version 460 core

layout(location=0) in vec3 aPosition;
layout(location=2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float scale;

out vec2 texCoords;

void main() {
    texCoords = ((aTexCoords * 2.0 - 1.0) * scale / 2.0) + 0.5;
    vec2 uv = clamp(texCoords, vec2(0.00001, 0.00001), vec2(0.99999, 0.99999));
    uv = uv * 2 - 1;
    uv.y = -uv.y;

    gl_Position = vec4(uv, 0.0, 1.0); 
}
