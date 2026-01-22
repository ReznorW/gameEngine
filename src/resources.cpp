#include <filesystem>
#include <iostream>

#include "resources.hpp"

Resources::Resources(std::string projectName) {
    loadAllMeshes(projectName);
    loadAllMaterials(projectName);
    loadAllScripts(projectName);
}

std::shared_ptr<Mesh> Resources::getMesh(const std::string& name) const {
    auto it = meshes.find(name);
    return it != meshes.end() ? it->second : nullptr;
}

std::shared_ptr<Material> Resources::getMaterial(const std::string& name) const {
    auto it = materials.find(name);
    if (it != materials.end()) {
        return it->second;
    } else {
        std::cout << "Could not find material: " << name << std::endl;
        return nullptr;
    }
}

std::shared_ptr<Script> Resources::getScript(const std::string& name) const {
    auto it = scripts.find(name);
    return it != scripts.end() ? it->second : nullptr;
}

std::vector<std::shared_ptr<Mesh>> Resources::getMeshes() const {
    std::vector<std::shared_ptr<Mesh>> result;
    for (const auto& [_, mesh] : meshes) result.push_back(mesh);
    return result;
}

std::vector<std::shared_ptr<Material>> Resources::getMaterials() const {
    std::vector<std::shared_ptr<Material>> result;
    for (const auto& [_, mat] : materials) result.push_back(mat);
    return result;
}

std::vector<std::shared_ptr<Script>> Resources::getScripts() const {
    std::vector<std::shared_ptr<Script>> result;
    for (const auto& [_, script] : scripts) result.push_back(script);
    return result;
}

std::vector<std::string> Resources::getSceneNames(std::string projectName) const {
    std::vector<std::string> names;
    for (const auto& entry : std::filesystem::directory_iterator("projects/" + projectName + "/scenes")) {
        if (entry.path().extension() == ".scn") {
            names.push_back(entry.path().stem().string());
        }
    }
    return names;
}

void Resources::addMesh(const std::shared_ptr<Mesh>& mesh) {
    if (mesh) meshes[mesh->getName()] = mesh;
}

void Resources::addMaterial(const std::shared_ptr<Material>& material) {
    if (material) materials[material->getName()] = material;
}

void Resources::addScript(const std::shared_ptr<Script>& script) {
    if (script) scripts[script->getName()] = script;
}

void Resources::deleteMesh(const std::string& name) {
    meshes.erase(name);
}

void Resources::deleteMaterial(const std::string& name) {
    materials.erase(name);
}

void Resources::deleteScript(const std::string& name) {
    scripts.erase(name);
}

void Resources::loadAllMeshes(std::string projectName) {
    std::cout << "===Loading in all meshes===" << std::endl;
    const std::string root = "projects/" + projectName + "/assets/models/";
    for (const auto& entry : std::filesystem::directory_iterator(root)) {
        if (entry.is_regular_file()) {
            std::string name = entry.path().stem().string();
            meshes[name] = std::make_shared<Mesh>(*loadObjFile(entry.path().string()));
            std::cout << "  - " << name << " loaded\n";
        }
    }
}

void Resources::loadAllMaterials(std::string projectName) {
    std::cout << "===Loading in all materials===\n";
    const std::string root = "projects/" + projectName + "/assets/materials/";
    for (const auto& entry : std::filesystem::directory_iterator(root)) {
        if (entry.is_directory()) {
            std::string name = entry.path().filename().string();
            materials[name] = std::make_shared<Material>(entry.path().string());
            std::cout << "  - " << name << " loaded\n";
        }
    }
}

void Resources::loadAllScripts(std::string projectName) {
    std::cout << "===Loading in all scripts===" << std::endl;
    const std::string root = "projects/" + projectName + "/scripts/";
    for (const auto& entry : std::filesystem::directory_iterator(root)) {
        if (entry.is_regular_file()) {
            std::string name = entry.path().stem().string();
            scripts[name] = std::make_shared<Script>(entry.path().string());
            std::cout << "  - " << name << " loaded\n";
        }
    }
}

void Resources::renameScript(const std::string& oldName, const std::string& newName) {
    auto it = scripts.find(oldName);
    if (it == scripts.end()) return;
    
    std::shared_ptr<Script> scriptPtr = it->second;
    scripts.erase(it);

    scriptPtr->setName(newName);

    scripts[newName] = scriptPtr;
}
