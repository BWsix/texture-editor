#version 460 core

layout(location=0) in vec3 aPosition;
layout(location=2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 texCoords;

void main() {
    mat4 mvp = projection * view * model;
    gl_Position = mvp * vec4(aPosition, 1.0);
    texCoords = aTexCoords;
}
