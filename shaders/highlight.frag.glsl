#version 460 core

layout(location=0) out vec4 fragColor;

uniform vec3 color;

in vec3 FragPos;
in vec3 Normal;

void main() {
    fragColor = vec4(color, 0.5);
}
