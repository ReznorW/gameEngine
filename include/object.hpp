#pragma once
#include <glm/glm.hpp>
#include "mesh.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "mesh.hpp"
#include "texture.hpp"

struct Transform {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    bool dirty = true;

    glm::mat4 getModelMatrix() const;
    bool needsUpdate() const;

    void markDirty() { dirty = true; }
    void markClean() { dirty = false; }
    

};

struct OBB {
    glm::vec3 center;
    glm::vec3 extents;
    glm::mat3 axes;
    
    OBB() : center(0.0f), extents(1.0f), axes(glm::mat3(1.0f)) {}
    OBB(const glm::vec3& min, const glm::vec3& max);
};

struct Object {
    std::string name;
    Mesh* mesh = nullptr;
    Shader* shader = nullptr;
    Texture* texture = nullptr;
    glm::vec2 textureScale = glm::vec2(1.0f, 1.0f);
    Transform transform;
    OBB obb;

    Object() = default;

    Object(const std::string& name, const std::string& modelName, const std::string& textureName, Shader* shader);
        
    void initializeOBB(const glm::vec3& meshMin, const glm::vec3& meshMax);
    void updateOBB();
    
    void draw(const Camera& camera) const;
};
