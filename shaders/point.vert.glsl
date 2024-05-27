#version 460 core

uniform vec3 pos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    mat4 mvp = projection * view * model;
    gl_Position = mvp * vec4(pos, 1.0);
}
