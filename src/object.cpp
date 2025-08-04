#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <iostream>
#include <ostream>

#include "object.hpp"
#include "camera.hpp"
#include "script.hpp"
#include "resources.hpp"
#include "scene.hpp"

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
    model *= glm::toMat4(rotationQuat);
    model = glm::scale(model, scale);

    return model;
}

void Transform::setFromModelMatrix(const glm::mat4& model) {
    glm::vec3 skew;
    glm::vec4 perspective;

    glm::decompose(model, scale, rotationQuat, position, skew, perspective);
    rotation = glm::degrees(glm::eulerAngles(rotationQuat));
}

void Transform::setRotation(const glm::vec3& degrees) {
    rotation = degrees;
    rotationQuat = glm::quat(glm::radians(rotation));
}

// ### Object functions ###
// === Constructor ===
Object::Object(const std::string& name, const std::string& meshName, const std::string& materialName, const std::string& scriptName, Resources* resources)
    : name(name) {
    mesh = resources->getMesh(meshName);
    material = resources->getMaterial(materialName);
    if (!scriptName.empty()) {
        auto baseScript = resources->getScript(scriptName);
        if (baseScript) {
            script = std::make_shared<Script>(*baseScript);
            script->setOwner(this);
        }
    }
}

Object::Object(const Object& other) :
    name(other.name), 
    isPlayer(other.isPlayer), 
    hasCollisions(other.hasCollisions), 
    isMoveable(other.isMoveable), 
    hasGravity(other.hasGravity), 
    pointLightID(other.pointLightID), 
    mesh(other.mesh), 
    material(other.material), 
    script(other.script ? std::make_shared<Script>(*other.script) : nullptr), 
    textureScale(other.textureScale), 
    transform(other.transform), 
    obb(other.obb), 
    parent(nullptr) {}

// === Deconstructor ===
Object::~Object() {}

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
    obb.extents = localExtents;
    
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

void getDescendants(Object* obj, std::vector<Object*>& out) {
    out.push_back(obj);
    for (Object* child : obj->children) {
        getDescendants(child, out);
    }
}

std::unique_ptr<Mesh> combineMeshes(const std::string& name, const std::vector<Object*>& objects) {
    std::vector<Vertex> combinedVertices;
    std::vector<unsigned int> combinedIndices;

    unsigned int indexOffset = 0;

    for (Object* obj : objects) {
        const glm::mat4 world = obj->getWorldMatrix();
        const std::vector<Vertex>& verts = obj->mesh->getVertices();
        const std::vector<unsigned int>& inds = obj->mesh->getIndices();

        for (const Vertex& v : verts) {
            Vertex transformed = v;
            glm::vec4 worldPos = world * glm::vec4(v.position, 1.0f);
            transformed.position = glm::vec3(worldPos);

            // Transform normals?
            combinedVertices.push_back(transformed);
        }

        for (unsigned int idx : inds) {
            combinedIndices.push_back(idx + indexOffset);
        }

        indexOffset += verts.size();
    }

    return std::make_unique<Mesh>(name, combinedVertices, combinedIndices);
}
