#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include "object.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "texture.hpp"

class Scene {
public:
    Scene();

    void init();
    std::vector<Mesh*> getMeshes() const;

    Shader* getShader(const std::string& name);
    std::vector<std::string> getShaderNames() const;

    Texture* getTexture(const std::string& name);
    std::vector<std::string> getTextureNames() const;
    std::vector<Texture*> getTextures() const;

    void addObject(const std::string& name, std::unique_ptr<Object> obj);
    Object* getObject(const std::string& name);
    std::vector<Object*> getObjects();
    size_t getObjectCount() const;
    void deleteObject(const std::string& name);
    void renameObject(const std::string& oldName, const std::string& newName);

    void draw(const Camera& camera);

    std::vector<std::string> getObjectNames() const;
    void clear();

    void selectObject(const std::string& name);
    Object* getSelectedObject() const;
    void clearSelection();

private:
    std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;
    void loadAllMeshes();

    std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;
    void loadAllShaders();

    std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
    void loadAllTextures();

    std::unordered_map<std::string, std::unique_ptr<Object>> objects;
    Object* selectedObject = nullptr;
};
