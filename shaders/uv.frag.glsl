#version 460 core

layout(location=0) out vec4 fragColor;

uniform vec3 color;

uniform sampler2D diffuse;
uniform sampler2D normal;
uniform sampler2D specular;

in vec2 texCoords;

void main() {
    fragColor = texture2D(diffuse, texCoords);
}
