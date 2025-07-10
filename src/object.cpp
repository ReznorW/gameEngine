#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <iostream>
#include <ostream>

#include "object.hpp"
#include "camera.hpp"
#include "script.hpp"
#include "resources.hpp"

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

// ### Object functions ###
// === Constructor ===
Object::Object(const std::string& name, const std::string& meshName, const std::string& textureName, const std::string& shaderName, const std::string& scriptName, Resources* resources)
    : name(name) {
    mesh = resources->getMesh(meshName);
    texture = resources->getTexture(textureName);
    shader = resources->getShader(shaderName);
    if (!scriptName.empty()) {
        auto baseScript = resources->getScript(scriptName);
        if (baseScript) {
            script = std::make_shared<Script>(*baseScript);
        }
    }
}

// Object::Object(const Object& other)
//     : name(other.name), isPlayer(other.isPlayer), mesh(other.mesh), shader(other.shader), texture(other.texture), script(other.script ? std::make_shared<Script>(*other.script) : nullptr), textureScale(other.textureScale), transform(other.transform), obb(other.obb), parent(nullptr) {}

Object::Object(const Object& other)
    : name(other.name), isPlayer(other.isPlayer), mesh(other.mesh), shader(other.shader), texture(other.texture), script(other.script), textureScale(other.textureScale), transform(other.transform), obb(other.obb), parent(nullptr) {}

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
