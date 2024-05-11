#version 460 core

layout(location=0) out vec4 color;
layout(location=1) out uvec4 id;

in vec2 texCoords;

void main() {
    color = vec4(vec3(gl_PrimitiveID % 20 * 0.03 + 0.4), 1.0);
    id = uvec4(gl_PrimitiveID + 1, 0, 0, 0);
}
