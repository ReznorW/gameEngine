#include <iostream>
#include <ostream>
#include <fstream>
#include <filesystem>

#include "scene.hpp"

// === Constructors ===
Scene::Scene() {
    loadAllMeshes();
    loadAllShaders();
    loadAllTextures();
}

Scene::Scene(const Scene& other) {
    // Map from original object pointer to cloned object pointer
    std::unordered_map<const Object*, Object*> pointerMap;

    // First pass: clone objects without parent/children set
    for (const auto& [name, obj] : other.objects) {
        auto cloned = std::make_unique<Object>();
        cloned->transform = obj->transform;
        cloned->name = obj->name;
        cloned->textureScale = obj->textureScale;
        cloned->obb = obj->obb;
        cloned->isPlayer = obj->isPlayer;

        cloned->mesh = obj->mesh;
        cloned->shader = obj->shader;
        cloned->texture = obj->texture;

        pointerMap[obj.get()] = cloned.get();
        objects[name] = std::move(cloned);
    }

    // Second pass: fix parent and children pointers
    for (const auto& [name, obj] : other.objects) {
        Object* clonedObj = pointerMap[obj.get()];
        // Fix parent
        if (obj->parent) {
            clonedObj->parent = pointerMap[obj->parent];
        } else {
            clonedObj->parent = nullptr;
        }

        // Fix children
        clonedObj->children.clear();
        for (Object* child : obj->children) {
            clonedObj->children.push_back(pointerMap[child]);
        }
    }

    // Fix selectedObject pointer
    if (other.selectedObject) {
        selectedObject = pointerMap[other.selectedObject];
    }

    name = other.name;
}

// === Mesh access ===
Mesh* Scene::getMesh(const std::string& name) const {
    auto mesh = meshes.find(name);
    if (mesh != meshes.end()) return mesh->second.get();
    return nullptr;
}

std::vector<Mesh*> Scene::getMeshes() const {
    std::vector<Mesh*> result;
    for (const auto& [name, mesh] : meshes) {
        result.push_back(mesh.get());
    }
    return result;
}

bool Scene::addMesh(std::unique_ptr<Mesh> mesh) {
    if (!mesh) return false;
    const std::string& name = mesh->getName();
    meshes[name] = std::move(mesh);
    return true;
}

bool Scene::removeMesh(const std::string& name) {
    auto it = meshes.find(name);
    if (it != meshes.end()) {
        meshes.erase(it);
        return true;
    }
    return false;
}

// === Shader access ===
Shader* Scene::getShader(const std::string& name) {
    auto it = shaders.find(name);
    if (it != shaders.end()) return it->second.get();
    return nullptr;
}

std::vector<std::string> Scene::getShaderNames() const {
    std::vector<std::string> names;
    for (const auto& [name, _] : shaders) {
        names.push_back(name);
    }
    return names;
}

// === Texture access ===
Texture* Scene::getTexture(const std::string& name) {
    if (textures.count(name)) {
        return textures[name].get();
    }
    return nullptr;
}

std::vector<Texture*> Scene::getTextures() const {
    std::vector<Texture*> result;
    for (const auto& [name, tex] : textures) {
        result.push_back(tex.get());
    }
    return result;
}

// === Scene handling ===
bool Scene::loadScene(const std::string& scnName) {
    clearSelection();
    clear();

    std::string fileName = "assets/scenes/" + scnName + ".scn";
    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cerr << "Failed to open scene file: " << scnName << std::endl;
        return false;
    }

    std::string line;
    std::string objName, meshName, textureName, shaderName, parentName = "None";
    glm::vec3 position(0), rotation(0), scale(1);
    glm::vec2 textureScale(1);
    bool isPlayer;
    bool inObjectBlock = false;
    std::unordered_map<std::string, std::unique_ptr<Object>> tempObjects;
    std::unordered_map<std::string, std::string> parentMap;

    // Parse .scn file
    setName(scnName);
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "object") {
            iss >> objName;
            meshName = shaderName = textureName = "";
            position = rotation = glm::vec3(0);
            scale = glm::vec3(1);
            textureScale = glm::vec2(1);
            isPlayer = false;
            parentName = "None";
            inObjectBlock = true;
        } else if (token == "mesh") {
            iss >> meshName;
        } else if (token == "shader") {
            iss >> shaderName;
        } else if (token == "texture") {
            iss >> textureName;
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
        } else if (token == "parent") {
            iss >> parentName;
        } else if (token == "endobject" && inObjectBlock) {
            auto obj = std::make_unique<Object>(objName, meshName, textureName, shaderName);
            obj->transform.position = position;
            obj->transform.rotation = rotation;
            obj->transform.scale = scale;
            obj->textureScale = textureScale;
            obj->isPlayer = isPlayer;

            tempObjects[objName] = std::move(obj);
            parentMap[objName] = parentName;

            inObjectBlock = false;
        }
    }

    // Fix parent pointers and children lists
    for (auto& [name, obj] : tempObjects) {
        std::string pName = parentMap[name];
        if (pName != "None" && tempObjects.count(pName)) {
            obj->parent = tempObjects[pName].get();
            obj->parent->children.push_back(obj.get());
        } else {
            obj->parent = nullptr;
        }
    }

    // Move objects into scene
    for (auto& [name, obj] : tempObjects) {
        addObject(name, std::move(obj));
    }

    return true;
}

bool Scene::saveScene(const std::string& scnName) {
    std::string fileName = "assets/scenes/" + scnName + ".scn";
    std::ofstream file(fileName);
    if (!file.is_open()) return false;

    // Write objects from scene to .scn file
    setName(scnName);
    for (const auto& objPtr : objects) {
        Object* obj = objPtr.second.get();
        file << "object " << obj->name << "\n";
        file << "mesh " << obj->mesh->getName() << "\n";
        file << "shader " << obj->shader->getName() << "\n";
        file << "texture " << obj->texture->getName() << "\n";
        file << "texturescale " << obj->textureScale.x << " " << obj->textureScale.y << "\n";
        file << "position " << obj->transform.position.x << " " << obj->transform.position.y << " " << obj->transform.position.z << "\n";
        file << "rotation " << obj->transform.rotation.x << " " << obj->transform.rotation.y << " " << obj->transform.rotation.z << "\n";
        file << "scale " << obj->transform.scale.x << " " << obj->transform.scale.y << " " << obj->transform.scale.z << "\n";
        file << "isPlayer " << obj->isPlayer << "\n";

        if (obj->parent) {
            file << "parent " << obj->parent->name << "\n";
        } else {
            file << "parent None\n";
        }

        file << "endobject\n\n";
    }

    file.close();
    return true;
}

std::vector<std::string> Scene::getSceneNames() const {
    std::vector<std::string> scenes;
    for (const auto& entry : std::filesystem::directory_iterator("assets/scenes")) {
        if (entry.is_regular_file() && entry.path().extension() == ".scn") {
            scenes.push_back(entry.path().filename().stem().string());
        }
    }
    return scenes;
}

void Scene::setName(const std::string& newName) {
    name = newName;
}

// === Object handling ===
void Scene::addObject(const std::string& name, std::unique_ptr<Object> obj) {
    objects[name] = std::move(obj);
}

Object* Scene::getObject(const std::string& name) {
    auto object = objects.find(name);
    if (object != objects.end()) {
        return object->second.get();
    }
    return nullptr;
}

std::vector<Object*> Scene::getObjects() {
    std::vector<Object*> result;
    for (auto& [name, objPtr] : objects) {
        result.push_back(objPtr.get());
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

size_t Scene::getObjectCount() const {
    return objects.size();
}

void Scene::deleteObject(const std::string& name) {
    objects.erase(name);
}

std::string Scene::duplicateObject(const std::string& originalName) {
    auto it = objects.find(originalName);
    if (it == objects.end()) {
        return "";
    }

    // Generate a unique name for the copy
    std::string baseName = originalName;
    std::string newName = baseName + "_copy";
    int suffix = 1;
    while (objects.count(newName) > 0) {
        newName = baseName + "_copy" + std::to_string(suffix++);
    }

    // Deep copy the object
    std::unique_ptr<Object> newObject = std::make_unique<Object>(*it->second);
    newObject->name = newName;

    // Insert into the scene
    objects[newName] = std::move(newObject);
    return newName;
}

std::string Scene::renameObject(const std::string& oldName, const std::string& newName) {
    auto it = objects.find(oldName);
    if (it == objects.end()) {
        return oldName;
    }

    std::string finalName = newName;
    int suffix = 1;
    while (objects.count(finalName) > 0 && finalName != oldName) {
        finalName = newName + "_" + std::to_string(suffix++);
    }

    objects[finalName] = std::move(it->second);
    objects.erase(it);

    objects[finalName]->name = finalName;

    if (selectedObject && selectedObject->name == oldName) {
        selectedObject = objects[finalName].get();
    }

    return finalName;
}

void Scene::clear() {
    objects.clear();
    setName("");
}

// === Selection handling ===
void Scene::selectObject(const std::string& name) {
    selectedObject = getObject(name);
}

Object* Scene::getSelectedObject() const {
    return selectedObject;
}

void Scene::clearSelection() {
    selectedObject = nullptr;
}

// === Rendering ===
void Scene::draw(const Camera& camera, bool inPlaytest) {
    for (const auto& [name, obj] : objects) {
        if (obj->parent) continue; // Only draw root objects
        
        obj->draw(camera, selectedObject, inPlaytest);
    }
}

// === Internal loaders ===
void Scene::loadAllMeshes() {
    std::cout << "===Loading in all meshes===" << std::endl;
    const std::string meshRoot = "assets/models";
    for (const auto& entry : std::filesystem::directory_iterator(meshRoot)) {
        if (entry.is_regular_file()) {
            std::string name = entry.path().stem().string();
            const std::string meshPath = meshRoot + "/" + name + ".vert";
            meshes[name] = std::make_unique<Mesh>(*loadVertFile(meshPath));
            std::cout << "    -" << name << " mesh loaded" << std::endl;
        }
    }
}

void Scene::loadAllShaders() {
    std::cout << "===Loading in all shaders===" << std::endl;
    const std::string shaderRoot = "assets/shaders";
    for (const auto& entry : std::filesystem::directory_iterator(shaderRoot)) {
        if (entry.is_directory()) {
            std::string name = entry.path().filename().string();

            std::string vertPath = entry.path().string() + "/vertex.glsl";
            std::string fragPath = entry.path().string() + "/fragment.glsl";

            if (std::filesystem::exists(vertPath) && std::filesystem::exists(fragPath)) {
                try {
                    auto shader = std::make_unique<Shader>(vertPath, fragPath, name);
                    shaders[name] = std::move(shader);
                    std::cout << "    -" << name << " shader loaded" << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Failed to load shader " << name << ": " << e.what() << std::endl;
                }
            } else {
                std::cerr << "Missing vertex/fragment in: " << entry.path() << std::endl;
            }
        }
    }
}

void Scene::loadAllTextures() {
    std::cout << "===Loading in all textures===" << std::endl;
    std::filesystem::path path = "assets/textures";
    for (auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_regular_file()) {
            std::string name = entry.path().stem().string();
            textures[name] = std::make_unique<Texture>(entry.path().string().c_str());
            std::cout << "    -" << name << " texture loaded" << std::endl;
        }
    }
}
