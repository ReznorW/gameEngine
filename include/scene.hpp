#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>

#include "camera.hpp"
#include "object.hpp"
#include "resources.hpp"

class Scene {
public:
    // Scene Properties
    glm::vec4 skyColor = glm::vec4(0.5f, 0.7f, 1.0f, 1.0f);
    glm::vec3 gravity = glm::vec3(0.0f, -15.0f, 0.0f);
    float drag = 0.8f;
    float playerSpeed = 1.0f;
    float playerJump = 10.0f;

    // Constructors
    explicit Scene(Resources* resources);
    Scene(const Scene& other);

    // Getters
    Resources* getResources() const {return resources;}
    std::string getName() const {return name;}
    Object* getObject(const std::string& name);
    std::vector<Object*> getObjects();
    std::vector<std::string> getObjectNames() const;
    int getObjectCount() const;
    Object* getPlayerObject() const;
    Object* getSelectedObject() const;

    // Setters
    void setName(const std::string& newName) {name = newName;}

    // Scene management
    bool loadScene(const std::string& name);
    bool saveScene(const std::string& name);

    // Object management
    void addObject(const std::string& name, std::shared_ptr<Object> obj);
    std::string duplicateObject(const std::string& name);
    void deleteObject(const std::string& name);
    void markForDeletion(const std::string& name);
    void processPendingDeletes();
    std::string renameObject(const std::string& oldName, const std::string& newName);
    void clear();

    // Selection
    void selectObject(const std::string& name);
    void clearSelection();

    // Drawing
    void draw(const Camera& camera, bool inPlaytest, bool drawOBBs);

private:
    std::string name;
    Resources* resources = nullptr;

    std::unordered_map<std::string, std::shared_ptr<Object>> objects;
    Object* selectedObject = nullptr;

    std::unordered_set<std::string> pendingDeletes;
};
