#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

#include "mesh.hpp"
#include "shader.hpp"
#include "material.hpp"
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

    // Materials
    std::shared_ptr<Material> getMaterial(const std::string& name) const;
    std::vector<std::shared_ptr<Material>> getMaterials() const;
    void addMaterial(const std::shared_ptr<Material>& material);
    void deleteMaterial(const std::string& name);

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
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;
    std::unordered_map<std::string, std::shared_ptr<Script>> scripts;

    // Loaders
    void loadAllMeshes(std::string projectName);
    void loadAllMaterials(std::string projectName);
    void loadAllScripts(std::string projectName);
};
