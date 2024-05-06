#include "shader.h"
#include "stb_include.h"

const char *keyword = "include";
const char *folder = "shaders";

void Shader::checkShaderCompileStatus(GLuint shader) {
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
        std::fprintf(stderr, "Failed to compile shader: %s\n", infoLog);
        exit(1);
    }
}

void Shader::checkProgramLinkStatus(GLuint program) {
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(program, sizeof(infoLog), NULL, infoLog);
        std::fprintf(stderr, "Failed to link shader program: %s\n", infoLog);
        exit(1);
    }
}

Shader& Shader::compile(const char *vertexPath, const char *fragmentPath) {
    ID = glCreateProgram();

    GLuint vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    char *vertexShaderSource = stb_include_file(
        (char *)vertexPath, (char *)keyword, (char *)folder, NULL);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkShaderCompileStatus(vertexShader);
    glAttachShader(ID, vertexShader);

    GLuint fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    char *fragmentShaderSource = stb_include_file(
        (char *)fragmentPath, (char *)keyword, (char *)folder, NULL);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkShaderCompileStatus(fragmentShader);
    glAttachShader(ID, fragmentShader);

    glLinkProgram(ID);
    checkProgramLinkStatus(ID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    free(vertexShaderSource);
    free(fragmentShaderSource);

    return *this;
}
