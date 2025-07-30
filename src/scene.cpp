#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>

#include "scene.hpp"
#include "obb.hpp"

// === Constructors ===
Scene::Scene() {}

Scene::Scene(const Scene& other) : 
    skyColor(other.skyColor), 
    gravity(other.gravity), 
    drag(other.drag), 
    playerSpeed(other.playerSpeed), 
    playerJump(other.playerJump), 
    name(other.name), 
    CSMFBO(other.CSMFBO), 
    omniFBO(other.omniFBO),
    CSMUBO(other.CSMUBO),
    CSMDepthMap(other.CSMDepthMap), 
    omniDepthCubeMap(other.omniDepthCubeMap),
    shader(other.shader), 
    CSMShader(other.CSMShader),
    omniShader(other.omniShader), 
    debugShader(other.debugShader) {

    std::unordered_map<const Object*, std::shared_ptr<Object>> pointerMap;

    // Clone objects
    for (const auto& [name, obj] : other.objects) {
        auto cloned = std::make_shared<Object>(*obj);
        pointerMap[obj.get()] = cloned;
        objects[name] = cloned;
    }

    // Fix parent/child hierarchy
    for (const auto& [name, obj] : other.objects) {
        auto cloned = pointerMap[obj.get()];
        cloned->parent = obj->parent ? pointerMap[obj->parent].get() : nullptr;
        cloned->children.clear();
        for (auto* child : obj->children) {
            cloned->children.push_back(pointerMap[child].get());
        }
    }

    if (other.selectedObject) {
        selectedObject = pointerMap[other.selectedObject].get();
    }
}

// === Getters ===
Object* Scene::getObject(const std::string& name) {
    auto it = objects.find(name);
    return (it != objects.end()) ? it->second.get() : nullptr;
}

std::vector<Object*> Scene::getObjects() {
    std::vector<Object*> result;
    for (const auto& [_, obj] : objects) {
        result.push_back(obj.get());
    }
    return result;
}

std::vector<std::string> Scene::getObjectNames() const {
    std::vector<std::string> names;
    for (const auto& [name, _] : objects) {
        names.push_back(name);
    }
    return names;
}

int Scene::getObjectCount() const {
    return objects.size();
}

Object* Scene::getPlayerObject() const {
    for (const auto& [_, obj] : objects) {
        if (obj->isPlayer) return obj.get();
    }
    return nullptr;
}

Object* Scene::getSelectedObject() const {
    return selectedObject;
}

// === Scene management ===
bool Scene::loadScene(const std::string& scnName, const Project& project, const Camera& camera) {
    clearSelection();
    clear();

    std::ifstream file("projects/" + project.name + "/scenes/" + scnName + ".scn");
    if (!file) {
        std::cerr << "Failed to open scene file: " << scnName << "\n";
        return false;
    }

    std::unordered_map<std::string, std::shared_ptr<Object>> tempObjects;
    std::unordered_map<std::string, std::string> parentMap;

    std::string line, objName, meshName, textureName, shaderName, scriptName, parentName = "None";
    glm::vec3 position, rotation, scale(1);
    glm::vec2 textureScale(1);
    float ambient, specular, shininess;
    bool isPlayer, hasCollisions, isMoveable, hasGravity, pointLight = false;

    setName(scnName);

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "scene") {
            skyColor = glm::vec4(0.5f, 0.7f, 1.0f, 1.0f);
            gravity = glm::vec3(0.0f, -15.0f, 0.0f);
            drag = 0.8f;
            playerSpeed = 1.0f;
            playerJump = 10.0f;
        } else if (token == "skycolor") {
            iss >> skyColor.x >> skyColor.y >> skyColor.z >> skyColor.w;
        } else if (token == "gravity") {
            iss >> gravity.x >> gravity.y >> gravity.z;
        } else if (token == "drag") {
            iss >> drag;
        } else if (token == "playerspeed") {
            iss >> playerSpeed;
        } else if (token == "playerjump") {
            iss >> playerJump;
        } else if (token == "object") {
            iss >> objName;
            meshName = textureName = scriptName = "";
            position = rotation = glm::vec3(0);
            scale = glm::vec3(1);
            textureScale = glm::vec2(1);
            isPlayer = false;
            pointLight = false;
            parentName = "None";
        } else if (token == "mesh") {
            iss >> meshName;
        } else if (token == "ambient") {
            iss >> ambient;
        } else if (token == "specular") {
            iss >> specular;
        } else if (token == "shininess") {
            iss >> shininess;
        } else if (token == "script") {
            iss >> scriptName;
        } else if (token == "texture") {
            iss >> textureName;
        } else if (token == "texturescale") {
            iss >> textureScale.x >> textureScale.y;
        } else if (token == "position") {
            iss >> position.x >> position.y >> position.z;
        } else if (token == "rotation") {
            iss >> rotation.x >> rotation.y >> rotation.z;
        } else if (token == "scale") {
            iss >> scale.x >> scale.y >> scale.z;
        } else if (token == "isPlayer") {
            iss >> isPlayer;
        } else if (token == "hasCollisions") {
            iss >> hasCollisions;
        } else if (token == "isMoveable") {
            iss >> isMoveable;
        } else if (token == "hasGravity") {
            iss >> hasGravity;
        } else if (token == "pointLight") {
            iss >> pointLight;
        } else if (token == "parent") {
            iss >> parentName;
        } else if (token == "endobject") {
            auto obj = std::make_shared<Object>(objName, meshName, textureName, scriptName, project.resources);
            obj->transform.position = position;
            obj->transform.setRotation(rotation);
            obj->transform.scale = scale;
            obj->transform.velocity = glm::vec3(0.0f);
            obj->material.ambient = ambient;
            obj->material.specular = specular;
            obj->material.shininess = shininess;
            obj->textureScale = textureScale;
            obj->isPlayer = isPlayer;
            obj->hasCollisions = hasCollisions;
            obj->isMoveable = isMoveable;
            obj->hasGravity = hasGravity;
            obj->pointLight = pointLight;

            tempObjects[objName] = obj;
            parentMap[objName] = parentName;
        }
    }

    for (auto& [name, obj] : tempObjects) {
        std::string pName = parentMap[name];
        if (pName != "None" && tempObjects.count(pName)) {
            obj->parent = tempObjects[pName].get();
            obj->parent->children.push_back(obj.get());
        }
        addObject(name, obj);
    }

    // Initialize shaders
    shader = std::make_shared<Shader>("default", false);
    CSMShader = std::make_shared<Shader>("CSM", true);
    omniShader = std::make_shared<Shader>("omni", true);
    debugShader = std::make_shared<Shader>("debug", false);

    // Initialize CSM
    CSMDepthMap = std::make_shared<Texture>(CSMShadowSize, camera.shadowCascadeLevels);
    glGenFramebuffers(1, &CSMFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, CSMFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, CSMDepthMap->getID(), 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Initialize Omni
    omniDepthCubeMap = std::make_shared<Texture>(OmniShadowSize);
    glGenFramebuffers(1, &omniFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, omniFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, omniDepthCubeMap->getID(), 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Initialize UBO
    glGenBuffers(1, &CSMUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, CSMUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 16, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, CSMUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    shader->use();
    shader->setInt("texture1", 0);
    shader->setInt("CSMDepthMap", 1);
    shader->setInt("omniDepthCubeMap", 2);

    return true;
}

bool Scene::saveScene(const std::string& scnName, const std::string& projectName) {
    std::ofstream file("projects/" + projectName + "/scenes/" + scnName + ".scn");
    if (!file.is_open()) return false;

    setName(scnName);

    file << "scene\n";
    file << "skycolor " << skyColor.x << " " << skyColor.y << " " << skyColor.z << " " << skyColor.w << "\n";
    file << "gravity " << gravity.x << " " << gravity.y << " " << gravity.z << "\n";
    file << "drag " << drag << "\n";
    file << "playerspeed " << playerSpeed << "\n";
    file << "playerjump " << playerJump << "\n";
    file << "endscene\n\n";

    for (const auto& [name, obj] : objects) {
        file << "object " << obj->name << "\n";
        file << "mesh " << obj->mesh->getName() << "\n";
        file << "ambient " << obj->material.ambient << "\n";
        file << "specular " << obj->material.specular << "\n";
        file << "shininess " << obj->material.shininess << "\n";
        if (obj->script) file << "script " << obj->script->getName() << "\n";
        file << "texture " << obj->texture->getName() << "\n";
        file << "texturescale " << obj->textureScale.x << " " << obj->textureScale.y << "\n";
        file << "position " << obj->transform.position.x << " " << obj->transform.position.y << " " << obj->transform.position.z << "\n";
        file << "rotation " << obj->transform.rotation.x << " " << obj->transform.rotation.y << " " << obj->transform.rotation.z << "\n";
        file << "scale " << obj->transform.scale.x << " " << obj->transform.scale.y << " " << obj->transform.scale.z << "\n";
        file << "isPlayer " << obj->isPlayer << "\n";
        file << "hasCollisions " << obj->hasCollisions << "\n";
        file << "isMoveable " << obj->isMoveable << "\n";
        file << "hasGravity " << obj->hasGravity << "\n";
        file << "pointLight " << obj->pointLight << "\n";
        file << "parent " << (obj->parent ? obj->parent->name : "None") << "\n";
        file << "endobject\n\n";
    }

    return true;
}

// === Object management ===
void Scene::addObject(const std::string& name, std::shared_ptr<Object> obj) {
    objects[name] = obj;
}

std::string Scene::duplicateObject(const std::string& originalName) {
    auto it = objects.find(originalName);
    if (it == objects.end()) return "";

    std::string newName = originalName + "_copy";
    int i = 1;
    while (objects.count(newName)) {
        newName = originalName + "_copy" + std::to_string(i++);
    }

    auto cloned = std::make_shared<Object>(*it->second);

    cloned->name = newName;
    if (cloned->isPlayer) {cloned->isPlayer = false;}
    cloned->name = newName;
    objects[newName] = cloned;
    return newName;
}

void Scene::deleteObject(const std::string& name) {
    objects.erase(name);
}

void Scene::markForDeletion(const std::string& name) {
    pendingDeletes.insert(name);
}

void Scene::processPendingDeletes() {
    for (const std::string& name : pendingDeletes) {
        deleteObject(name);
    }
    pendingDeletes.clear();
}

std::string Scene::renameObject(const std::string& oldName, const std::string& newName) {
    auto it = objects.find(oldName);
    if (it == objects.end()) return oldName;

    std::string finalName = newName;
    int i = 1;
    while (objects.count(finalName) && finalName != oldName) {
        finalName = newName + "_" + std::to_string(i++);
    }

    auto obj = it->second;
    obj->name = finalName;
    objects.erase(it);
    objects[finalName] = obj;

    if (selectedObject && selectedObject->name == oldName) {
        selectedObject = obj.get();
    }

    return finalName;
}

void Scene::clear() {
    objects.clear();
    selectedObject = nullptr;
    skyColor = glm::vec4(0.5f, 0.7f, 1.0f, 1.0f);
    gravity = glm::vec3(0.0f, -15.0f, 0.0f);
    drag = 0.8f;
    playerSpeed = 1.0f;
    playerJump = 10.0f;
}

// === Selection ===
void Scene::selectObject(const std::string& name) {
    selectedObject = getObject(name);
}

void Scene::clearSelection() {
    selectedObject = nullptr;
}

// === Draw ===
void Scene::draw(const Context& context, Camera& camera, bool inPlaytest, bool drawOBBs) {
    float ambient = 0.10;

    // UBO setup
    const auto lightMatrices = camera.getLightSpaceMatrices(glm::normalize(lightDir));
    glBindBuffer(GL_UNIFORM_BUFFER, CSMUBO);
    for (size_t i = 0; i < lightMatrices.size(); ++i) {
        glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &lightMatrices[i]);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Depth cubemap transform matrices setup
    float pointLightNear = 1.0f;
    float pointLightFar = 25.0f;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, pointLightNear, pointLightFar);
    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

    // CSM pass
    CSMShader->use();
    glBindFramebuffer(GL_FRAMEBUFFER, CSMFBO);
    glViewport(0, 0, CSMShadowSize, CSMShadowSize);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
    for (const auto& [_, obj] : objects) {
        CSMShader->setMat4("model", obj->getWorldMatrix());
        obj->mesh->draw();
    }
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Omni pass
    omniShader->use();
    glBindFramebuffer(GL_FRAMEBUFFER, omniFBO);
    glViewport(0, 0, OmniShadowSize, OmniShadowSize);
    glClear(GL_DEPTH_BUFFER_BIT);
    for (unsigned int i = 0; i < 6; i++) {
        omniShader->setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
    }
    omniShader->setFloat("far", pointLightFar);
    omniShader->setVec3("lightPos", lightPos);
    for (const auto& [_, obj] : objects) {
        omniShader->setMat4("model", obj->getWorldMatrix());
        obj->mesh->draw();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glViewport(0, 0, context.window->getWidth(), context.window->getHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Final pass
    shader->use();
    // Camera uniforms
    shader->setMat4("projection", camera.getProjectionMatrix());
    shader->setMat4("view", camera.getViewMatrix());
    // Lighting uniforms
    shader->setVec3("viewPos", camera.getPosition());
    shader->setVec3("lightPos", lightPos);
    shader->setVec3("lightDir", glm::normalize(lightDir));
    shader->setFloat("cameraFar", camera.far);
    shader->setFloat("pointFar", pointLightFar);
    shader->setInt("cascadeCount", camera.shadowCascadeLevels.size());
    for (size_t i = 0; i < camera.shadowCascadeLevels.size(); ++i) {
        shader->setFloat("cascadePlaneDistances[" + std::to_string(i) + "]", camera.shadowCascadeLevels[i]);
    }
    shader->setFloat("ambient", ambient);
    // Fog uniforms
    shader->setVec3("fogColor", glm::vec3(0.5f, 0.6f, 0.7f));
    shader->setFloat("fogStart", 50.0f);
    shader->setFloat("fogEnd", 100.0f);
    // Object uniforms
    for (const auto& [_, obj] : objects) {
        obj->draw(*this, inPlaytest);
    }

    if (drawOBBs) {
        for (const auto& [_, obj] : objects) {
            drawOBB(obj->obb, camera, debugShader.get(), glm::vec3(1.0f, 0.0f, 0.0f));
        }
    }
}
