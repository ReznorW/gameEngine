#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

#include "object.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "texture.hpp"

// Scene definition
class Scene {
public:
    // Constructor
    Scene();

    // Mesh access
    std::vector<Mesh*> getMeshes() const;

    // Shader access
    Shader* getShader(const std::string& name);
    std::vector<std::string> getShaderNames() const;

    // Texture access
    Texture* getTexture(const std::string& name);
    std::vector<Texture*> getTextures() const;

    // Object handling
    void addObject(const std::string& name, std::unique_ptr<Object> obj);
    Object* getObject(const std::string& name);
    std::vector<Object*> getObjects();
    std::vector<std::string> getObjectNames() const;
    size_t getObjectCount() const;
    void deleteObject(const std::string& name);
    std::string duplicateObject(const std::string& originalName);
    std::string renameObject(const std::string& oldName, const std::string& newName);
    void clear();

    // Selection handling
    void selectObject(const std::string& name);
    Object* getSelectedObject() const;
    void clearSelection();

    // Rendering
    void draw(const Camera& camera);

private:
    // Resource containers
    std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;
    std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;
    std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
    std::unordered_map<std::string, std::unique_ptr<Object>> objects;

    Object* selectedObject = nullptr;

    // Internal loaders
    void loadAllMeshes();
    void loadAllShaders();
    void loadAllTextures();
};
