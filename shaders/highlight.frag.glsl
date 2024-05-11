#version 460 core

layout(location=0) out vec4 fragColor;

uniform vec3 color;

in vec2 texCoords;

void main() {
    fragColor = vec4(texCoords, 0.0, 1.0);
}
