#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>

#include "camera.hpp"
#include "object.hpp"
#include "resources.hpp"
#include "project.hpp"
#include "window.hpp"

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    float near;
    float far;
    GLuint depthCubeMap;
    GLuint FBO;
};

class Scene {
public:
    // Scene Properties
    glm::vec4 skyColor = glm::vec4(0.5f, 0.7f, 1.0f, 1.0f);
    glm::vec3 gravity = glm::vec3(0.0f, -15.0f, 0.0f);
    float drag = 0.8f;
    float playerSpeed = 1.0f;
    float playerJump = 10.0f;
    glm::vec3 lightDir = glm::vec3(20.0f, 50, 20.0f);
    glm::vec3 lightPos = glm::vec3(0.0f, 5.0f, 0.0f);

    // Constructors
    Scene();
    Scene(const Scene& other);

    // Getters
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
    bool loadScene(const std::string& name, const Project& project, const Camera& camera);
    bool saveScene(const std::string& name, const std::string& projectName);

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
    void draw(const Context& context, Camera& camera, bool inPlaytest, bool drawOBBs);
    Shader* getShader() {return shader.get();}
    Texture* getCSMDepthMap() {return CSMDepthMap.get();}
    Texture* getOmniDepthCubeMap() {return omniDepthCubeMap.get();}

private:
    std::string name;

    std::unordered_map<std::string, std::shared_ptr<Object>> objects;
    Object* selectedObject = nullptr;
    std::unordered_set<std::string> pendingDeletes;

    // === Lighting ===
    // Constants
    const unsigned int CSMShadowSize = 2048;
    const unsigned int OmniShadowSize = 1024;

    // Lights
    //std::vector<PointLight> pointLights;

    // FBOs
    unsigned int CSMFBO;
    unsigned int omniFBO;

    // UBOs
    unsigned int CSMUBO;

    // Depth maps
    std::shared_ptr<Texture> CSMDepthMap;
    std::shared_ptr<Texture> omniDepthCubeMap;

    // Shaders
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Shader> CSMShader;
    std::shared_ptr<Shader> omniShader;
    std::shared_ptr<Shader> debugShader;
};
