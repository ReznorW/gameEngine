#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>

#include "scene.hpp"
#include "obb.hpp"

// === Constructors ===
Scene::Scene(Resources* resources)
    : resources(resources) {}

Scene::Scene(const Scene& other)
    : name(other.name), resources(other.resources) {

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
bool Scene::loadScene(const std::string& scnName) {
    clearSelection();
    clear();

    std::ifstream file("assets/scenes/" + scnName + ".scn");
    if (!file) {
        std::cerr << "Failed to open scene file: " << scnName << "\n";
        return false;
    }

    std::unordered_map<std::string, std::shared_ptr<Object>> tempObjects;
    std::unordered_map<std::string, std::string> parentMap;

    std::string line, objName, meshName, textureName, shaderName, scriptName, parentName = "None";
    glm::vec3 position, rotation, scale(1);
    glm::vec2 textureScale(1);
    bool isPlayer, hasCollisions, isMoveable, hasGravity = false;
    bool inObject = false;

    setName(scnName);

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "object") {
            iss >> objName;
            meshName = shaderName = textureName = scriptName = "";
            position = rotation = glm::vec3(0);
            scale = glm::vec3(1);
            textureScale = glm::vec2(1);
            isPlayer = false;
            parentName = "None";
            inObject = true;
        } else if (token == "mesh") {
            iss >> meshName;
        } else if (token == "shader") {
            iss >> shaderName;
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
        } else if (token == "parent") {
            iss >> parentName;
        } else if (token == "endobject" && inObject) {
            auto obj = std::make_shared<Object>(objName, meshName, textureName, shaderName, scriptName, resources);
            obj->transform.position = position;
            obj->transform.rotation = rotation;
            obj->transform.scale = scale;
            obj->transform.velocity = glm::vec3(0.0f);
            obj->textureScale = textureScale;
            obj->isPlayer = isPlayer;
            obj->hasCollisions = hasCollisions;
            obj->isMoveable = isMoveable;
            obj->hasGravity = hasGravity;

            tempObjects[objName] = obj;
            parentMap[objName] = parentName;
            inObject = false;
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

    return true;
}

bool Scene::saveScene(const std::string& scnName) {
    std::ofstream file("assets/scenes/" + scnName + ".scn");
    if (!file.is_open()) return false;

    setName(scnName);

    for (const auto& [name, obj] : objects) {
        file << "object " << obj->name << "\n";
        file << "mesh " << obj->mesh->getName() << "\n";
        file << "shader " << obj->shader->getName() << "\n";
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
}

// === Selection ===
void Scene::selectObject(const std::string& name) {
    selectedObject = getObject(name);
}

void Scene::clearSelection() {
    selectedObject = nullptr;
}

// === Draw ===
void Scene::draw(const Camera& camera, bool inPlaytest, bool drawOBBs) {
    for (const auto& [_, obj] : objects) {
        if (obj->parent) continue;
        obj->draw(camera, selectedObject, inPlaytest); 
    }

    if (drawOBBs) {
        auto debugShader = resources->getShader("debug");
        for (const auto& [_, obj] : objects) {
            drawOBB(obj->obb, camera, debugShader.get(), glm::vec3(1.0f, 0.0f, 0.0f));
        }
    }
}
