#pragma once

#include "glad/gl.h"
#include "shader.h"
#include "stb_image.h"
#include <string>

class Texture {
public:
    unsigned int diffuse = 0;
    unsigned int normal = 0;
    unsigned int specular = 0;

    Texture() {
        glGenTextures(1, &this->diffuse);
        glGenTextures(1, &this->normal);
        glGenTextures(1, &this->specular);
    }
    void Bind(Shader prog) const {
        prog.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->diffuse);
        prog.setInt("diffuse", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, this->normal);
        prog.setInt("normal", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, this->specular);
        prog.setInt("specular", 2);

        glActiveTexture(GL_TEXTURE0);
    }

    static Texture Load(const std::string& path);
    static void read(std::string filename, GLuint target);

};
