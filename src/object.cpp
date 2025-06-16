#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <iostream>
#include <ostream>

#include "object.hpp"
#include "camera.hpp"

// ### Transform functions ###
// === Update handling ===
bool Transform::needsUpdate() const {
    if (dirty) {
        return true;
    }
    return false;
}

// === Get transformed model ===
glm::mat4 Transform::getModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));
    model = glm::scale(model, scale);

    return model;
}

void Transform::setFromModelMatrix(const glm::mat4& model) {
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::quat rotationQuat;

    glm::decompose(model, scale, rotationQuat, position, skew, perspective);
    rotation = glm::degrees(glm::eulerAngles(rotationQuat));
}

// ### OBB functions ###
// === Constructor ===
OBB::OBB(const glm::vec3& min, const glm::vec3& max) 
    : center((min + max) * 0.5f), extents(max - center), axes(glm::mat3(1.0f)) {}

// ### Object functions ###
// === Constructor ===
Object::Object(const std::string& name, const std::string& modelName, const std::string& textureName, const std::string& shaderName)
    : name(name) {
    std::string modelPath = "assets/models/" + modelName + ".vert";
    mesh = loadVertFile(modelPath);
    std::string texturePath = "assets/textures/" + textureName + ".jpg";
    texture = new Texture(texturePath);
    std::string vertPath = "assets/shaders/" + shaderName + "/vertex.glsl";
    std::string fragPath = "assets/shaders/" + shaderName + "/fragment.glsl";
    shader = new Shader(vertPath, fragPath, shaderName);
    Object::initializeOBB(mesh->getMinBounds(), mesh->getMaxBounds());
} 

Object::Object(const Object& other)
    : name(other.name), mesh(other.mesh), shader(other.shader), texture(other.texture), textureScale(other.textureScale), transform(other.transform), obb(other.obb), parent(nullptr), children() {}

// === OBB handling ===
void Object::initializeOBB(const glm::vec3& meshMin, const glm::vec3& meshMax) {
    obb = OBB(meshMin, meshMax);
}

void Object::updateOBB() {
    if (!mesh) return;

    // Get the model matrix from transform
    const glm::mat4 modelMatrix = getWorldMatrix();
    
    // Calculate local center (before transform)
    glm::vec3 localCenter = (mesh->getMinBounds() + mesh->getMaxBounds()) * 0.5f;
    
    // Transform center to world space
    obb.center = glm::vec3(modelMatrix * glm::vec4(localCenter, 1.0f));
    
    // Handle scaling and rotation:
    // 1. Extract rotation matrix (normalized axes)
    obb.axes = glm::mat3(modelMatrix);
    for (int i = 0; i < 3; i++) {
        obb.axes[i] = glm::normalize(obb.axes[i]);
    }
    
    // 2. Apply scale to extents (in local space)
    glm::vec3 localExtents = mesh->getMaxBounds() - localCenter;
    obb.extents = localExtents * transform.scale;
    
    // 3. Transform extents to account for rotation
    // (This handles non-uniform scaling correctly)
    obb.extents.x *= glm::length(glm::vec3(modelMatrix[0]));
    obb.extents.y *= glm::length(glm::vec3(modelMatrix[1]));
    obb.extents.z *= glm::length(glm::vec3(modelMatrix[2]));
}

// === Inheritance handling ===
glm::mat4 Object::getWorldMatrix() const {
    if (parent) {
        return parent->getWorldMatrix() * transform.getModelMatrix();
    } else {
        return transform.getModelMatrix();
    }
}

void Object::setParent(Object* newParent) {
    glm::mat4 worldMatrix = getWorldMatrix();

    if (parent) {
        auto& siblings = parent->children;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
    }

    parent = newParent;

    if (newParent) {
        newParent->children.push_back(this);
    }

    glm::mat4 parentWorldInverse = newParent ? glm::inverse(newParent->getWorldMatrix()) : glm::mat4(1.0f);
    glm::mat4 localMatrix = parentWorldInverse * worldMatrix;
    transform.setFromModelMatrix(localMatrix);
    transform.markDirty();
}

bool Object::isDescendant(const Object* target) const {
    for (const Object* child : children) {
        if (child == target || child->isDescendant(target)) {
            return true;
        }
    }
    return false;
}

// === Rendering ===
void Object::draw(const Camera& camera, const Object* selectedObject, const bool inPlaytest) const {
    bool isHighlighted = (this == selectedObject) || (selectedObject && selectedObject->isDescendant(this));

    if (!(inPlaytest && isPlayer)) {
        shader->use();

        // Set 3D model
        shader->setMat4("model", getWorldMatrix());
        shader->setMat4("view", camera.getViewMatrix());
        shader->setMat4("projection", camera.getProjectionMatrix());

        // Set lighting params
        shader->setVec3("viewPos", camera.getPosition());
        shader->setVec3("lightDir", glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f))); // Sunlight from above
        shader->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f)); // White sunlight
        shader->setVec3("fogColor", glm::vec3(0.5f, 0.6f, 0.7f)); // Adjust to your desired fog color
        shader->setFloat("fogStart", 50.0f);  // Distance where fog starts
        shader->setFloat("fogEnd", 100.0f);   // Distance where fog fully saturates
        shader->setBool("isSelected", isHighlighted);    // Whether or not object is selected

        // Set texture
        if (texture) {
            texture->bind(0);
            shader->setInt("texture1", 0);
            shader->setVec2("textureScale", textureScale);
        }

        mesh->draw();
    }

    for (const Object* child : children) {
        child->draw(camera, selectedObject, inPlaytest);
    }
}
