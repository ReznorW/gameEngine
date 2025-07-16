#include <filesystem>
#include <iostream>

#include "resources.hpp"

Resources::Resources() {
    loadAllMeshes();
    loadAllShaders();
    loadAllTextures();
    loadAllScripts();
}

std::shared_ptr<Mesh> Resources::getMesh(const std::string& name) const {
    auto it = meshes.find(name);
    return it != meshes.end() ? it->second : nullptr;
}

std::shared_ptr<Shader> Resources::getShader(const std::string& name) const {
    auto it = shaders.find(name);
    return it != shaders.end() ? it->second : nullptr;
}

std::shared_ptr<Texture> Resources::getTexture(const std::string& name) const {
    auto it = textures.find(name);
    return it != textures.end() ? it->second : nullptr;
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

std::vector<std::shared_ptr<Shader>> Resources::getShaders() const {
    std::vector<std::shared_ptr<Shader>> result;
    for (const auto& [_, shader] : shaders) result.push_back(shader);
    return result;
}

std::vector<std::shared_ptr<Texture>> Resources::getTextures() const {
    std::vector<std::shared_ptr<Texture>> result;
    for (const auto& [_, tex] : textures) result.push_back(tex);
    return result;
}

std::vector<std::shared_ptr<Script>> Resources::getScripts() const {
    std::vector<std::shared_ptr<Script>> result;
    for (const auto& [_, script] : scripts) result.push_back(script);
    return result;
}

std::vector<std::string> Resources::getSceneNames() const {
    std::vector<std::string> names;
    for (const auto& entry : std::filesystem::directory_iterator("assets/scenes")) {
        if (entry.path().extension() == ".scn") {
            names.push_back(entry.path().stem().string());
        }
    }
    return names;
}

void Resources::addMesh(const std::shared_ptr<Mesh>& mesh) {
    if (mesh) meshes[mesh->getName()] = mesh;
}

void Resources::addShader(const std::shared_ptr<Shader>& shader) {
    if (shader) shaders[shader->getName()] = shader;
}

void Resources::addTexture(const std::shared_ptr<Texture>& texture) {
    if (texture) textures[texture->getName()] = texture;
}

void Resources::addScript(const std::shared_ptr<Script>& script) {
    if (script) scripts[script->getName()] = script;
}

void Resources::deleteMesh(const std::string& name) {
    meshes.erase(name);
}

void Resources::deleteShader(const std::string& name) {
    shaders.erase(name);
}

void Resources::deleteTexture(const std::string& name) {
    textures.erase(name);
}

void Resources::deleteScript(const std::string& name) {
    scripts.erase(name);
}

void Resources::loadAllMeshes() {
    std::cout << "===Loading in all meshes===" << std::endl;
    const std::string root = "assets/models/";
    for (const auto& entry : std::filesystem::directory_iterator(root)) {
        if (entry.is_regular_file()) {
            std::string name = entry.path().stem().string();
            meshes[name] = std::make_shared<Mesh>(*loadVertFile(entry.path().string()));
            std::cout << "  - " << name << " loaded\n";
        }
    }
}

void Resources::loadAllShaders() {
    std::cout << "===Loading in all shaders===" << std::endl;
    const std::string root = "assets/shaders/";
    for (const auto& entry : std::filesystem::directory_iterator(root)) {
        if (entry.is_directory()) {
            std::string name = entry.path().filename().string();
            std::string vert = entry.path().string() + "/vertex.glsl";
            std::string frag = entry.path().string() + "/fragment.glsl";
            shaders[name] = std::make_shared<Shader>(vert, frag, name);
            std::cout << "  - " << name << " loaded\n";
        }
    }
}

void Resources::loadAllTextures() {
    std::cout << "===Loading in all textures===" << std::endl;
    const std::string root = "assets/textures/";
    for (const auto& entry : std::filesystem::directory_iterator(root)) {
        if (entry.is_regular_file()) {
            std::string name = entry.path().filename().string();
            textures[name] = std::make_shared<Texture>(entry.path().string());
            std::cout << "  - " << name << " loaded\n";
        }
    }
}

void Resources::loadAllScripts() {
    std::cout << "===Loading in all scripts===" << std::endl;
    const std::string root = "assets/scripts/";
    for (const auto& entry : std::filesystem::directory_iterator(root)) {
        if (entry.is_regular_file()) {
            std::string name = entry.path().stem().string();
            scripts[name] = std::make_shared<Script>(entry.path().string());
            std::cout << "  - " << name << " loaded\n";
        }
    }
}
