#pragma once

#include <glm/glm.hpp>
#include <memory>

#include "mesh.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "obb.hpp"

// Forward declarations
class Script;
class Resources;

// Transform definition
struct Transform {
    // Transform vectors
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::quat rotationQuat = glm::quat();
    glm::vec3 scale = glm::vec3(1.0f);
    glm::vec3 velocity = glm::vec3(0.0f);

    // Update handling
    bool dirty = true;
    void markDirty() {dirty = true;}
    void markClean() {dirty = false;}
    bool needsUpdate() const;

    // Get transformed model
    glm::mat4 getModelMatrix() const;
    void setFromModelMatrix(const glm::mat4& model);
    void setRotation(const glm::vec3& degrees);
};

// Material definition
struct Material {
    float ambient = 0.2f;
    float specular = 0.5f;
    float shininess = 32.0f;
};

// Object definition
struct Object {
    // Object data
    std::string name;
    bool isPlayer = false;
    bool hasCollisions = false;
    bool isMoveable = false;
    bool hasGravity = false;
    bool isGrounded = false;

    std::shared_ptr<Mesh> mesh = nullptr;
    std::shared_ptr<Shader> shader = nullptr;
    std::shared_ptr<Texture> texture = nullptr;
    std::shared_ptr<Script> script = nullptr;

    glm::vec2 textureScale = glm::vec2(1.0f, 1.0f);

    Transform transform;
    Material material;
    OBB obb;

    Object* parent = nullptr;
    std::vector<Object*> children;

    // Constructors
    Object() = default;
    Object(const std::string& name, const std::string& modelName, const std::string& textureName, const std::string& shaderName, const std::string& scriptName, Resources* resources);
    Object(const Object& other);

    // Deconstructor
    ~Object();
        
    // OBB handling
    void initializeOBB(const glm::vec3& meshMin, const glm::vec3& meshMax);
    void updateOBB();

    // Inheritance handling
    glm::mat4 getWorldMatrix() const;
    void setParent(Object* newParent);
    bool isDescendant(const Object* target) const;
    
    // Rendering
    void draw(const Camera& camera, const Object* selectedObject, const bool inPlaytest) const;
};

void getDescendants(Object* obj, std::vector<Object*>& out);
std::unique_ptr<Mesh> combineMeshes(const std::string& name, const std::vector<Object*>& objects);
