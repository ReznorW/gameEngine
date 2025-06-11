#include <iostream>
#include <ostream>
#include <filesystem>

#include "scene.hpp"

// === Constructor ===
Scene::Scene() {}

// === Initialization ===
void Scene::init() {
    loadAllMeshes();
    loadAllShaders();
    loadAllTextures();
}

// === Mesh access ===
std::vector<Mesh*> Scene::getMeshes() const {
    std::vector<Mesh*> result;
    for (const auto& [name, mesh] : meshes) {
        result.push_back(mesh.get());
    }
    return result;
}

// === Shader access ===
Shader* Scene::getShader(const std::string& name) {
    auto it = shaders.find(name);
    if (it != shaders.end()) return it->second.get();
    return nullptr;
}

std::vector<std::string> Scene::getShaderNames() const {
    std::vector<std::string> names;
    for (const auto& [name, _] : shaders) {
        names.push_back(name);
    }
    return names;
}

// === Texture access ===
Texture* Scene::getTexture(const std::string& name) {
    if (textures.count(name)) {
        return textures[name].get();
    }
    return nullptr;
}

std::vector<Texture*> Scene::getTextures() const {
    std::vector<Texture*> result;
    for (const auto& [name, tex] : textures) {
        result.push_back(tex.get());
    }
    return result;
}

// === Object handling ===
void Scene::addObject(const std::string& name, std::unique_ptr<Object> obj) {
    objects[name] = std::move(obj);
}

Object* Scene::getObject(const std::string& name) {
    auto object = objects.find(name);
    if (object != objects.end()) {
        return object->second.get();
    }
    return nullptr;
}

std::vector<Object*> Scene::getObjects() {
    std::vector<Object*> result;
    for (auto& [name, objPtr] : objects) {
        result.push_back(objPtr.get());
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

size_t Scene::getObjectCount() const {
    return objects.size();
}

void Scene::deleteObject(const std::string& name) {
    objects.erase(name);
}

std::string Scene::duplicateObject(const std::string& originalName) {
    auto it = objects.find(originalName);
    if (it == objects.end()) {
        return "";
    }

    // Generate a unique name for the copy
    std::string baseName = originalName;
    std::string newName = baseName + "_copy";
    int suffix = 1;
    while (objects.count(newName) > 0) {
        newName = baseName + "_copy" + std::to_string(suffix++);
    }

    // Deep copy the object
    std::unique_ptr<Object> newObject = std::make_unique<Object>(*it->second);
    newObject->name = newName;

    // Insert into the scene
    objects[newName] = std::move(newObject);
    return newName;
}

std::string Scene::renameObject(const std::string& oldName, const std::string& newName) {
    auto it = objects.find(oldName);
    if (it == objects.end()) {
        return oldName;
    }

    std::string finalName = newName;
    int suffix = 1;
    while (objects.count(finalName) > 0 && finalName != oldName) {
        finalName = newName + "_" + std::to_string(suffix++);
    }

    objects[finalName] = std::move(it->second);
    objects.erase(it);

    objects[finalName]->name = finalName;

    if (selectedObject && selectedObject->name == oldName) {
        selectedObject = objects[finalName].get();
    }

    return finalName;
}

void Scene::clear() {
    objects.clear();
}

// === Selection handling ===
void Scene::selectObject(const std::string& name) {
    selectedObject = getObject(name);
}

Object* Scene::getSelectedObject() const {
    return selectedObject;
}

void Scene::clearSelection() {
    selectedObject = nullptr;
}

// === Rendering ===
void Scene::draw(const Camera& camera) {
    for (auto& obj : objects) {
        obj.second->draw(camera);
    }
}

// === Internal loaders ===
void Scene::loadAllMeshes() {
    std::cout << "===Loading in all meshes===" << std::endl;
    const std::string meshRoot = "assets/models";
    for (const auto& entry : std::filesystem::directory_iterator(meshRoot)) {
        if (entry.is_regular_file()) {
            std::string name = entry.path().stem().string();
            const std::string meshPath = meshRoot + "/" + name + ".vert";
            meshes[name] = std::make_unique<Mesh>(*loadVertFile(meshPath));
            std::cout << "    -" << name << " mesh loaded" << std::endl;
        }
    }
}

void Scene::loadAllShaders() {
    std::cout << "===Loading in all shaders===" << std::endl;
    const std::string shaderRoot = "assets/shaders";
    for (const auto& entry : std::filesystem::directory_iterator(shaderRoot)) {
        if (entry.is_directory()) {
            std::string name = entry.path().filename().string();

            std::string vertPath = entry.path().string() + "/vertex.glsl";
            std::string fragPath = entry.path().string() + "/fragment.glsl";

            if (std::filesystem::exists(vertPath) && std::filesystem::exists(fragPath)) {
                try {
                    auto shader = std::make_unique<Shader>(vertPath, fragPath, name);
                    shaders[name] = std::move(shader);
                    std::cout << "    -" << name << " shader loaded" << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Failed to load shader " << name << ": " << e.what() << std::endl;
                }
            } else {
                std::cerr << "Missing vertex/fragment in: " << entry.path() << std::endl;
            }
        }
    }
}

void Scene::loadAllTextures() {
    std::cout << "===Loading in all textures===" << std::endl;
    std::filesystem::path path = "assets/textures";
    for (auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_regular_file()) {
            std::string name = entry.path().stem().string();
            textures[name] = std::make_unique<Texture>(entry.path().string().c_str());
            std::cout << "    -" << name << " texture loaded" << std::endl;
        }
    }
}
