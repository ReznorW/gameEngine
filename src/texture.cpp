#include <glad/glad.h>
#include <stb_image.h>
#include <iostream>
#include <vector>

#include "texture.hpp"

// === Constructor ===
Texture::Texture(const std::string& path) {
    name = path.substr(path.find_last_of("/\\") + 1);
    stbi_set_flip_vertically_on_load(true);

    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return;
    }

    GLenum format = GL_RGB;
    if (channels == 4) format = GL_RGBA;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(data);
}

Texture::Texture(unsigned int& depthMapFBO, const unsigned int shadowSize, const std::vector<float>& shadowCascadeLevels) {
    name = "depthMap";
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, id);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, shadowSize, shadowSize, int(shadowCascadeLevels.size()) + 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);	

    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
}

// === Deconstructor ===
Texture::~Texture() {
    if (id) {
        glDeleteTextures(1, &id);
    }
}

// === Usage ===
void Texture::bind(unsigned int slot) const {
    if (id == 0) {
        std::cerr << "Warning: Trying to bind texture with id=0\n";
        return;
    }
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::bindArray(unsigned int slot) const {
    if (id == 0) {
        std::cerr << "Warning: Trying to bind texture with id=0\n";
        return;
    }
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D_ARRAY, id);
}
