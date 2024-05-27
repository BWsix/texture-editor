#include "texture.h"
#include <iostream>

Texture Texture::Load(const std::string &filename) {
    std::cout << "Loading Texture: " + filename + "\n";

    Texture texture;
    read(filename, texture.diffuse);

    return texture;
}

void Texture::read(std::string filename, GLuint target) {
    int width, height, nrComponents;
    GLenum internalFormat = GL_RGB;
    GLenum format = GL_RGB;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

    if (data) {
        if (nrComponents == 1) {
            internalFormat = GL_RED;
            format = GL_RED;
        } else if (nrComponents == 3) {
            internalFormat = GL_RGB;
            format = GL_RGB;
            std::cout << "RGB\n";
        } else if (nrComponents == 4) {
            internalFormat = GL_RGBA;
            format = GL_RGBA;
            std::cout << "RGBA\n";
        }

        glBindTexture(GL_TEXTURE_2D, target);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << filename << std::endl;
        stbi_image_free(data);
    }
}
