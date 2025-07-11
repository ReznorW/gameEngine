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
    // Constructors
    explicit Scene(Resources* resources);
    Scene(const Scene& other);

    Resources* getResources() const {return resources;}

    // Scene management
    bool loadScene(const std::string& name);
    bool saveScene(const std::string& name);
    std::vector<std::string> getSceneNames() const;
    void setName(const std::string& name);
    std::string getName() const {return name;}

    // Object management
    void addObject(const std::string& name, std::shared_ptr<Object> obj);
    Object* getObject(const std::string& name);
    Object* getPlayerObject() const;
    std::vector<Object*> getObjects();
    std::vector<std::string> getObjectNames() const;
    size_t getObjectCount() const;
    void markForDeletion(const std::string& name);
    void deleteObject(const std::string& name);
    void processPendingDeletes();
    std::string duplicateObject(const std::string& name);
    std::string renameObject(const std::string& oldName, const std::string& newName);
    void clear();

    // Selection
    void selectObject(const std::string& name);
    Object* getSelectedObject() const;
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
