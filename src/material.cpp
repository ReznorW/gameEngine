#include <glad/glad.h>
#include <stb_image.h>
#include <iostream>
#include <vector>
#include <filesystem>
#include <algorithm>

#include "material.hpp"

// === Constructor ===
Material::Material(const std::string& materialPath) {
    name = std::filesystem::path(materialPath).filename().string();
    stbi_set_flip_vertically_on_load(true);

    std::string diffusePath;

    for (const auto& entry : std::filesystem::directory_iterator(materialPath)) {
        std::string filename = entry.path().filename().string();
        std::string lower = filename;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (lower.find("diffuse.") == 0) {
            diffusePath = entry.path().string();
        }
    }

    if (!diffusePath.empty()) {
        diffuseTexture = loadTexture(diffusePath);
    }

    specular = 1.0f;
    shininess = 32.0f;
}

// === Deconstructor ===
Material::~Material() {
    if (diffuseTexture) {glDeleteTextures(1, &diffuseTexture);}
}

// === Loader ===
unsigned int Material::loadTexture(const std::string& path) {
    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return 0;
    }

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(data);
    return textureID;
}

// === Usage ===
void Material::bind(unsigned int diffuseSlot) const {
    if (diffuseTexture) {
        glActiveTexture(GL_TEXTURE0 + diffuseSlot);
        glBindTexture(GL_TEXTURE_2D, diffuseTexture);
    }
}
