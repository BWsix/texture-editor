#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

class Shader {
    uint ID; 

public:
    Shader& compile(const char *vertexPath, const char *fragmentPath);
    void use() const { glUseProgram(ID); }

    void setBool(const std::string &name, bool value) const { glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); }
    void setInt(const std::string &name, int value) const { glUniform1i(glGetUniformLocation(ID, name.c_str()), value); }
    void setFloat(const std::string &name, float value) const { glUniform1f(glGetUniformLocation(ID, name.c_str()), value); }
    void setVec2(const std::string &name, const glm::vec2 &value) const { glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value)); }
    void setVec3(const std::string &name, const glm::vec3 &value) const { glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value)); }
    void setVec4(const std::string &name, const glm::vec4 &value) const { glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value)); }
    void setMat4(const std::string &name, const glm::mat4 &mat) const { glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat)); }

private:
    void checkShaderCompileStatus(unsigned int shader);
    void checkProgramLinkStatus(unsigned int program);
};
