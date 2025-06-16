#pragma once

#include <glm/glm.hpp>

#include "mesh.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "mesh.hpp"
#include "texture.hpp"

// Transform definition
struct Transform {
    // Transform vectors
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);

    // Update handling
    bool dirty = true;
    void markDirty() {dirty = true;}
    void markClean() {dirty = false;}
    bool needsUpdate() const;

    // Get transformed model
    glm::mat4 getModelMatrix() const;
    void setFromModelMatrix(const glm::mat4& model);
};

// Oriented Bounding Box (OBB) definition
struct OBB {
    // OBB vectors
    glm::vec3 center;
    glm::vec3 extents;
    glm::mat3 axes;
    
    // Constructors
    OBB() : center(0.0f), extents(1.0f), axes(glm::mat3(1.0f)) {}
    OBB(const glm::vec3& min, const glm::vec3& max);
};

// Object definition
struct Object {
    // Object data
    std::string name;
    bool isPlayer = false;

    Mesh* mesh = nullptr;
    Shader* shader = nullptr;
    Texture* texture = nullptr;
    glm::vec2 textureScale = glm::vec2(1.0f, 1.0f);

    Transform transform;
    OBB obb;

    Object* parent = nullptr;
    std::vector<Object*> children;

    // Constructors
    Object() = default;
    Object(const std::string& name, const std::string& modelName, const std::string& textureName, const std::string& shaderName);
    Object(const Object& other);
        
    // OBB handling
    void initializeOBB(const glm::vec3& meshMin, const glm::vec3& meshMax);
    void updateOBB();

    // Inheritance handling
    glm::mat4 getWorldMatrix() const;
    void setParent(Object* newParent);
    bool isDescendant(const Object* target) const;
    void updateChildren();
    
    // Rendering
    void draw(const Camera& camera, const Object* selectedObject, const bool inPlaytest) const;
};
