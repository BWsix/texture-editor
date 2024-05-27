#version 460 core

layout(location=0) out vec4 fragColor;

uniform vec3 color;
uniform sampler2D tex;

in vec2 texCoords;

void main() {
    fragColor = vec4(1);
}
