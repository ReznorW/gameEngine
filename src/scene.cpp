#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>

#include "scene.hpp"
#include "obb.hpp"

// === Constructors ===
Scene::Scene(const Camera& camera) {
    renderer = new Renderer(camera);
}

Scene::Scene(const Scene& other) : 
    skyColor(other.skyColor), 
    gravity(other.gravity), 
    drag(other.drag), 
    playerSpeed(other.playerSpeed), 
    playerJump(other.playerJump), 
    ambient(other.ambient),
    renderer(other.renderer),
    name(other.name) {

    std::unordered_map<const Object*, std::shared_ptr<Object>> pointerMap;

    // Clone objects
    for (const auto& [name, obj] : other.objects) {
        auto cloned = std::make_shared<Object>(*obj);
        pointerMap[obj.get()] = cloned;
        objects[name] = cloned;
    }

    // Fix parent/child hierarchy
    for (const auto& [name, obj] : other.objects) {
        auto cloned = pointerMap[obj.get()];
        cloned->parent = obj->parent ? pointerMap[obj->parent].get() : nullptr;
        cloned->children.clear();
        for (auto* child : obj->children) {
            cloned->children.push_back(pointerMap[child].get());
        }
    }

    if (other.selectedObject) {
        selectedObject = pointerMap[other.selectedObject].get();
    }
}

// === Getters ===
Object* Scene::getObject(const std::string& name) {
    auto it = objects.find(name);
    return (it != objects.end()) ? it->second.get() : nullptr;
}

std::vector<Object*> Scene::getObjects() {
    std::vector<Object*> result;
    for (const auto& [_, obj] : objects) {
        result.push_back(obj.get());
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

int Scene::getObjectCount() const {
    return objects.size();
}

Object* Scene::getPlayerObject() const {
    for (const auto& [_, obj] : objects) {
        if (obj->isPlayer) return obj.get();
    }
    return nullptr;
}

Object* Scene::getSelectedObject() const {
    return selectedObject;
}

// === Scene management ===
bool Scene::loadScene(const std::string& scnName, const Project& project, const Camera& camera) {
    clearSelection();
    clear();

    std::ifstream file("projects/" + project.name + "/scenes/" + scnName + ".scn");
    if (!file) {
        std::cerr << "Failed to open scene file: " << scnName << "\n";
        return false;
    }

    renderer = new Renderer(camera);

    std::unordered_map<std::string, std::shared_ptr<Object>> tempObjects;
    std::unordered_map<std::string, std::string> parentMap;

    std::string line, objName, meshName, materialName, shaderName, scriptName, parentName = "None";
    glm::vec3 position, rotation, scale(1);
    glm::vec2 textureScale(1);
    int pointLightID;
    bool isPlayer, hasCollisions, isMoveable, hasGravity = false;
    glm::vec3 lightColor(1.0f);
    float lightIntensity = 1.0f;
    float lightNear = 0.1f;
    float lightFar = 25.0f;
     float ambient = 0.15f;

    setName(scnName);

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "scene") {
            skyColor = glm::vec4(0.5f, 0.7f, 1.0f, 1.0f);
            renderer->getDirectionalLight().color = glm::vec3(1.0f, 1.0f, 1.0f);
            renderer->getDirectionalLight().direction = glm::vec3(20.0f, -50.0f, 20.0f);
            renderer->getDirectionalLight().intensity = 0.5f;
            gravity = glm::vec3(0.0f, -15.0f, 0.0f);
            drag = 0.8f;
            playerSpeed = 1.0f;
            playerJump = 10.0f;
            ambient = 0.15f;
        } else if (token == "skycolor") {
            iss >> skyColor.x >> skyColor.y >> skyColor.z >> skyColor.w;
        } else if (token == "sunlightColor") {
            iss >> renderer->getDirectionalLight().color.x >> renderer->getDirectionalLight().color.y >> renderer->getDirectionalLight().color.z;
        } else if (token == "sunlightDir") {
            iss >> renderer->getDirectionalLight().direction.x >> renderer->getDirectionalLight().direction.y >> renderer->getDirectionalLight().direction.z;
        } else if (token == "sunlightIntensity") {
            iss >> renderer->getDirectionalLight().intensity;
        } else if (token == "gravity") {
            iss >> gravity.x >> gravity.y >> gravity.z;
        } else if (token == "drag") {
            iss >> drag;
        } else if (token == "playerspeed") {
            iss >> playerSpeed;
        } else if (token == "playerjump") {
            iss >> playerJump;
        } else if (token == "object") {
            iss >> objName;
            meshName = materialName = scriptName = "";
            position = rotation = glm::vec3(0);
            scale = glm::vec3(1);
            textureScale = glm::vec2(1);
            isPlayer = false;
            pointLightID = -1;
            lightColor = glm::vec3(1.0f);
            lightIntensity = 1.0f;
            lightNear = 0.1f;
            lightFar = 25.0f;
            parentName = "None";
        } else if (token == "mesh") {
            iss >> meshName;
        } else if (token == "ambient") {
            iss >> ambient;
        } else if (token == "script") {
            iss >> scriptName;
        } else if (token == "material") {
            iss >> materialName;
        } else if (token == "texturescale") {
            iss >> textureScale.x >> textureScale.y;
        } else if (token == "position") {
            iss >> position.x >> position.y >> position.z;
        } else if (token == "rotation") {
            iss >> rotation.x >> rotation.y >> rotation.z;
        } else if (token == "scale") {
            iss >> scale.x >> scale.y >> scale.z;
        } else if (token == "isPlayer") {
            iss >> isPlayer;
        } else if (token == "hasCollisions") {
            iss >> hasCollisions;
        } else if (token == "isMoveable") {
            iss >> isMoveable;
        } else if (token == "hasGravity") {
            iss >> hasGravity;
        } else if (token == "pointLightID") {
            iss >> pointLightID;
        } else if (token == "lightColor") {
            iss >> lightColor.r >> lightColor.g >> lightColor.b;
        } else if (token == "lightIntensity") {
            iss >> lightIntensity;
        } else if (token == "lightNear") {
            iss >> lightNear;
        } else if (token == "lightFar") {
            iss >> lightFar;
        } else if (token == "parent") {
            iss >> parentName;
        } else if (token == "endobject") {
            auto obj = std::make_shared<Object>(objName, meshName, materialName, scriptName, project.resources);
            obj->transform.position = position;
            obj->transform.setRotation(rotation);
            obj->transform.scale = scale;
            obj->transform.velocity = glm::vec3(0.0f);
            obj->textureScale = textureScale;
            obj->isPlayer = isPlayer;
            obj->hasCollisions = hasCollisions;
            obj->isMoveable = isMoveable;
            obj->hasGravity = hasGravity;
            obj->pointLightID = pointLightID;
            if (obj->pointLightID > -1) {
                PointLight light;
                light.position = obj->transform.position;
                light.color = lightColor;
                light.intensity = lightIntensity;
                light.near = lightNear;
                light.far = lightFar;

                int newID = renderer->addPointLight(light);
                obj->pointLightID = newID;
            }

            tempObjects[objName] = obj;
            parentMap[objName] = parentName;
        }
    }

    for (auto& [name, obj] : tempObjects) {
        std::string pName = parentMap[name];
        if (pName != "None" && tempObjects.count(pName)) {
            obj->parent = tempObjects[pName].get();
            obj->parent->children.push_back(obj.get());
        }
        addObject(name, obj);
    }

    return true;
}

bool Scene::saveScene(const std::string& scnName, const std::string& projectName) {
    std::ofstream file("projects/" + projectName + "/scenes/" + scnName + ".scn");
    if (!file.is_open()) return false;

    setName(scnName);

    file << "scene\n";
    file << "skycolor " << skyColor.x << " " << skyColor.y << " " << skyColor.z << " " << skyColor.w << "\n";
    file << "sunlightColor " << renderer->getDirectionalLight().color.x << " " << renderer->getDirectionalLight().color.y << " " << renderer->getDirectionalLight().color.z << "\n";
    file << "sunlightDir " << renderer->getDirectionalLight().direction.x << " " << renderer->getDirectionalLight().direction.y << " " << renderer->getDirectionalLight().direction.z << "\n";
    file << "sunlightIntensity " << renderer->getDirectionalLight().intensity << "\n";
    file << "ambient " << ambient << "\n";
    file << "gravity " << gravity.x << " " << gravity.y << " " << gravity.z << "\n";
    file << "drag " << drag << "\n";
    file << "playerspeed " << playerSpeed << "\n";
    file << "playerjump " << playerJump << "\n";
    file << "endscene\n\n";

    for (const auto& [name, obj] : objects) {
        file << "object " << obj->name << "\n";
        file << "mesh " << obj->mesh->getName() << "\n";
        if (obj->script) file << "script " << obj->script->getName() << "\n";
        file << "material " << obj->material->getName() << "\n";
        file << "texturescale " << obj->textureScale.x << " " << obj->textureScale.y << "\n";
        file << "position " << obj->transform.position.x << " " << obj->transform.position.y << " " << obj->transform.position.z << "\n";
        file << "rotation " << obj->transform.rotation.x << " " << obj->transform.rotation.y << " " << obj->transform.rotation.z << "\n";
        file << "scale " << obj->transform.scale.x << " " << obj->transform.scale.y << " " << obj->transform.scale.z << "\n";
        file << "isPlayer " << obj->isPlayer << "\n";
        file << "hasCollisions " << obj->hasCollisions << "\n";
        file << "isMoveable " << obj->isMoveable << "\n";
        file << "hasGravity " << obj->hasGravity << "\n";
        file << "pointLightID " << obj->pointLightID << "\n";
        if (obj->pointLightID > -1) {
            PointLight light = renderer->getPointLight(obj->pointLightID);
            file << "lightColor " << light.color.x << " " << light.color.y << " " << light.color.z << "\n";
            file << "lightIntensity " << light.intensity << "\n";
            file << "lightNear " << light.near << "\n";
            file << "lightFar " << light.far << "\n";
        }
        file << "parent " << (obj->parent ? obj->parent->name : "None") << "\n";
        file << "endobject\n\n";
    }

    return true;
}

// === Object management ===
void Scene::addObject(const std::string& name, std::shared_ptr<Object> obj) {
    objects[name] = obj;
}

std::string Scene::duplicateObject(const std::string& originalName) {
    auto it = objects.find(originalName);
    if (it == objects.end()) return "";

    std::string newName = originalName + "_copy";
    int i = 1;
    while (objects.count(newName)) {
        newName = originalName + "_copy" + std::to_string(i++);
    }

    auto cloned = std::make_shared<Object>(*it->second);

    cloned->name = newName;
    if (cloned->isPlayer) {cloned->isPlayer = false;}
    if (cloned->pointLightID > -1) {
        PointLight light = renderer->getPointLight(it->second->pointLightID);
        cloned->pointLightID = renderer->addPointLight(light);
    }
    cloned->name = newName;
    objects[newName] = cloned;
    return newName;
}

void Scene::deleteObject(const std::string& name) {
    renderer->removePointLight(objects.find(name)->second->pointLightID);
    objects.erase(name);
}

void Scene::markForDeletion(const std::string& name) {
    pendingDeletes.insert(name);
}

void Scene::processPendingDeletes() {
    for (const std::string& name : pendingDeletes) {
        deleteObject(name);
    }
    pendingDeletes.clear();
}

std::string Scene::renameObject(const std::string& oldName, const std::string& newName) {
    auto it = objects.find(oldName);
    if (it == objects.end()) return oldName;

    std::string finalName = newName;
    int i = 1;
    while (objects.count(finalName) && finalName != oldName) {
        finalName = newName + "_" + std::to_string(i++);
    }

    auto obj = it->second;
    obj->name = finalName;
    objects.erase(it);
    objects[finalName] = obj;

    if (selectedObject && selectedObject->name == oldName) {
        selectedObject = obj.get();
    }

    return finalName;
}

void Scene::clear() {
    objects.clear();
    selectedObject = nullptr;
    skyColor = glm::vec4(0.5f, 0.7f, 1.0f, 1.0f);
    gravity = glm::vec3(0.0f, -15.0f, 0.0f);
    drag = 0.8f;
    playerSpeed = 1.0f;
    playerJump = 10.0f;
}

// === Selection ===
void Scene::selectObject(const std::string& name) {
    selectedObject = getObject(name);
}

void Scene::clearSelection() {
    selectedObject = nullptr;
}

// === Draw ===
void Scene::draw(Context& context, Camera& camera, bool inPlaytest) {
    if (renderer->drawDirectionalShadows) {
        renderer->renderDirectionalLight(camera, getObjects());
    }
    if (renderer->drawPointShadows) {
        for (int i = 0; i < static_cast<int>(renderer->getPointLights().size()); i++) {
            renderer->renderPointLight(i, getObjects());
        }
    }
    renderer->renderScene(context, *this, camera, inPlaytest);
    if (renderer->drawOBBs) {
        renderer->renderOBBs(camera, getObjects());
    }
}
