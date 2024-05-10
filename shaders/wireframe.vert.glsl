#version 460 core

layout(location=0) in vec3 aPosition;
layout(location=3) in uint aFaceID;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

flat out uint faceID;

void main() {
    mat4 mvp = projection * view * model;
    gl_Position = mvp * vec4(aPosition, 1.0);
    faceID = aFaceID;
}
