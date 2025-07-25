#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "script.hpp"

class Resources {
public:
    // Constructor
    Resources(std::string projectName);

    // Meshes
    std::shared_ptr<Mesh> getMesh(const std::string& name) const;
    std::vector<std::shared_ptr<Mesh>> getMeshes() const;
    void addMesh(const std::shared_ptr<Mesh>& mesh);
    void deleteMesh(const std::string& name);

    // Shaders
    std::shared_ptr<Shader> getShader(const std::string& name) const;
    std::vector<std::shared_ptr<Shader>> getShaders() const;
    void addShader(const std::shared_ptr<Shader>& shader);
    void deleteShader(const std::string& name);

    // Textures
    std::shared_ptr<Texture> getTexture(const std::string& name) const;
    std::vector<std::shared_ptr<Texture>> getTextures() const;
    void addTexture(const std::shared_ptr<Texture>& texture);
    void deleteTexture(const std::string& name);

    // Scripts
    std::shared_ptr<Script> getScript(const std::string& name) const;
    std::vector<std::shared_ptr<Script>> getScripts() const;
    void addScript(const std::shared_ptr<Script>& script);
    void deleteScript(const std::string& name);
    void renameScript(const std::string& oldName, const std::string& newName);

    // Scenes
    std::vector<std::string> getSceneNames(std::string projectName) const;

private:
    // Resource containers
    std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes;
    std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
    std::unordered_map<std::string, std::shared_ptr<Script>> scripts;

    // Loaders
    void loadAllMeshes(std::string projectName);
    void loadAllShaders(std::string projectName);
    void loadAllTextures(std::string projectName);
    void loadAllScripts(std::string projectName);
};
