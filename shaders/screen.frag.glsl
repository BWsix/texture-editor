#version 430 core

in vec2 TexCoord;

layout(location=0) out vec4 FragColor;

uniform sampler2D screenTex;

void main() {
    FragColor = texture(screenTex, TexCoord);
}
