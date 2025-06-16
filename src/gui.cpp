#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glm/gtc/type_ptr.hpp>

#include "gui.hpp"
#include "mode.hpp"

// === Constructor ===
Gui::Gui(Window& window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;
    io.BackendFlags &= ~ImGuiBackendFlags_HasMouseCursors;

    ImGui_ImplGlfw_InitForOpenGL(window.getGLFWwindow(), false);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();
}

// === Shutdown ===
void Gui::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

// === Frame lifecycle ===
void Gui::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Gui::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// === Input syncing ===
void Gui::syncMouseFromGLFW(GLFWwindow* window) {
    ImGuiIO& io = ImGui::GetIO();
    
    // Get mouse position from GLFW
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    io.MousePos = ImVec2((float)mouseX, (float)mouseY);
    
    // Get mouse buttons from GLFW
    io.MouseDown[0] = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    io.MouseDown[1] = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    io.MouseDown[2] = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
}

void Gui::syncKeyboardFromGLFW(GLFWwindow* window) {
    ImGuiIO& io = ImGui::GetIO();
    
    // Synchronize modifier keys
    io.KeyCtrl = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) || 
                 (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);
    io.KeyShift = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) || 
                  (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
    io.KeyAlt = (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) || 
                (glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS);
    io.KeySuper = (glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS) || 
                  (glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS);
}

// === Rendering ===
void Gui::drawMainMenu(Window& window, Scene& scene, std::unique_ptr<Scene>& playScene, Camera& camera, Camera& playCamera, Mode& mode) {
    static bool openLoadScenePopup = false;
    static bool openSaveScenePopup = false;

    if (ImGui::BeginMainMenuBar()) {
        // File Menu
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New")) {
                scene.clear();
            }
            if (ImGui::MenuItem("Load")) {
                openLoadScenePopup = true; 
            }
            if (ImGui::MenuItem("Save As")) {
                openSaveScenePopup = true;
            }
            if (ImGui::MenuItem("Save")) {
                // TODO: Implement quick saving
            }
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(window.getGLFWwindow(), true);
            }
            ImGui::EndMenu();
        }

        // Edit Menu
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("New Object", "C")) {
                std::string objName = "NewObj" + std::to_string(scene.getObjectCount());
                scene.addObject(objName, std::make_unique<Object>(objName, "cube", "default", "default"));
                scene.selectObject(objName);
            }
            if (ImGui::MenuItem("Undo")) {
                // TODO: Implement undo stack
            }
            if (ImGui::MenuItem("Redo")) {
                // TODO: Implement redo stack
            }
            ImGui::EndMenu();
        }

        // Selection Menu
        if (ImGui::BeginMenu("Selection")) {
            if (ImGui::MenuItem("Deselect", "Escape")) {
                scene.clearSelection();
            }
            if (ImGui::MenuItem("Duplicate Selection", "X")) {
                Object* selected = scene.getSelectedObject();
                if (selected) {
                    std::string newName = scene.duplicateObject(selected->name);
                    if (!newName.empty()) {
                        scene.selectObject(newName);
                    }
                }
            }
            ImGui::EndMenu();
        }

        // Run Menu
        if (ImGui::BeginMenu("Run")) {
            if (ImGui::MenuItem("Playtest", "R")) {
                if (mode == Mode::Editor) {
                    mode = Mode::Playtest;
                    playScene = std::make_unique<Scene>(scene);
                    playScene->clearSelection();
                    for (auto& obj : playScene->getObjects()) {
                        if (obj->isPlayer) {
                            playCamera.position = obj->transform.position; //+ glm::vec3(0.0f, 0.0f, 0.0f); // TODO: Dynamically change camera position for object
                            playCamera.yaw = -obj->transform.rotation.y;
                            playCamera.pitch = obj->transform.rotation.x;
                            playCamera.updateCameraVectors();
                        }
                    }
                }
            }
            ImGui::EndMenu();
        }

        // FPS counter
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 100.0f);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

        ImGui::EndMainMenuBar();
    }

    // Show object properties if one is selected
    if (Object* selected = scene.getSelectedObject()) {
        drawObjectProperties(scene, selected);
    }

    if (openLoadScenePopup) {
        ImGui::OpenPopup("Load Scene Popup");
        openLoadScenePopup = false;
    }
    drawLoadScenePopup(scene);

    if (openSaveScenePopup) {
        ImGui::OpenPopup("Save Scene Popup");
        openSaveScenePopup = false;
    }
    drawSaveScenePopup(scene);
}

void Gui::drawSidebar(Scene& scene) {
    ImGui::SetNextWindowPos(ImVec2(0, 20));
    ImGui::SetNextWindowSize(ImVec2(200, ImGui::GetIO().DisplaySize.y - 20));
    ImGui::Begin("Objects", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    for (auto& obj : scene.getObjects()) {
        if (obj->parent == nullptr) {  // Only draw root objects
            drawObjectTree(*obj, scene);
        }
    }

    ImGui::End();
}

void Gui::drawObjectTree(Object& obj, Scene& scene) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (scene.getSelectedObject() == &obj) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    bool nodeOpen = false;
    if (!obj.children.empty()) {
        nodeOpen = ImGui::TreeNodeEx(obj.name.c_str(), flags);
    } else {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        ImGui::TreeNodeEx(obj.name.c_str(), flags);
    }

    if (ImGui::IsItemClicked()) {
        scene.selectObject(obj.name);
    }

    if (nodeOpen) {
        for (Object* child : obj.children) {
            drawObjectTree(*child, scene);  // Recursive call
        }
        ImGui::TreePop();
    }
}

void Gui::drawObjectProperties(Scene& scene, Object* selected) {
    if (ImGui::Begin("Object Properties")) {
        // Editable Name
        char nameBuffer[128];
        std::strncpy(nameBuffer, selected->name.c_str(), sizeof(nameBuffer));
        nameBuffer[sizeof(nameBuffer) - 1] = '\0'; // Ensure null-termination

        ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer));
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            std::string newName(nameBuffer);
            if (!newName.empty() && newName != selected->name) {
                std::string finalName = scene.renameObject(selected->name, newName);
                selected->name = finalName;
            }
        }

        // Transform controls
        ImGui::DragFloat3("Position", glm::value_ptr(selected->transform.position), 0.1f);
        ImGui::DragFloat3("Rotation", glm::value_ptr(selected->transform.rotation), 0.1f);
        ImGui::DragFloat3("Scale",    glm::value_ptr(selected->transform.scale),    0.1f);
        selected->transform.markDirty();

        // Parent selector
        std::string currentParentName = selected->parent ? selected->parent->name : "None";
        if (ImGui::BeginCombo("Parent", currentParentName.c_str())) {
            // Option to clear the parent
            if (ImGui::Selectable("None", selected->parent == nullptr)) {
                // Detach from current parent
                if (selected->parent) {
                    auto& siblings = selected->parent->children;
                    siblings.erase(std::remove(siblings.begin(), siblings.end(), selected), siblings.end());
                    selected->parent = nullptr;
                }
            }

            // List all other objects as potential parents
            for (Object* potentialParent : scene.getObjects()) {
                if (potentialParent == selected) continue;

                if (selected->isDescendant(potentialParent)) continue;

                bool isSelected = (selected->parent == potentialParent);
                if (ImGui::Selectable(potentialParent->name.c_str(), isSelected)) {
                    selected->setParent(potentialParent);
                }
            }

            ImGui::EndCombo();
        }

        // Mesh selector
        std::string currentMesh = selected->mesh ? selected->mesh->getName() : "None";

        if (ImGui::BeginCombo("Mesh", currentMesh.c_str())) {
            auto meshes = scene.getMeshes();
            for (Mesh* mesh : meshes) {
                const std::string& meshName = mesh->getName();
                bool isSelected = (meshName == currentMesh);
                if (ImGui::Selectable(meshName.c_str(), isSelected)) {
                    selected->mesh = mesh;
                    selected->initializeOBB(mesh->getMinBounds(), mesh->getMaxBounds());
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        // Shader selector
        std::string currentShader = selected->shader ? selected->shader->getName() : "None";

        if (ImGui::BeginCombo("Shader", currentShader.c_str())) {
            auto shaderNames = scene.getShaderNames();
            for (const auto& shaderName : shaderNames) {
                bool isSelected = (shaderName == currentShader);
                if (ImGui::Selectable(shaderName.c_str(), isSelected)) {
                    selected->shader = scene.getShader(shaderName);
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        // Texture selector
        std::string currentTextureName = selected->texture ? selected->texture->getName() : "None";

        if (ImGui::BeginCombo("Texture", currentTextureName.c_str())) {
            auto textures = scene.getTextures();
            for (Texture* tex : textures) {
                const std::string& texName = tex->getName();
                bool isSelected = (selected->texture == tex);
                if (ImGui::Selectable(texName.c_str(), isSelected)) {
                    selected->texture = tex;
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Text("Texture Scale");
        float scale[2] = { selected->textureScale.x, selected->textureScale.y };
        if (ImGui::InputFloat("Scale X", &scale[0], 0.01f, 1.0f, "%.3f")) {
            selected->textureScale.x = scale[0];
        }
        if (ImGui::InputFloat("Scale Y", &scale[1], 0.01f, 1.0f, "%.3f")) {
            selected->textureScale.y = scale[1];
        }
    }

    if (ImGui::Checkbox("Player", &selected->isPlayer)) {
        if (selected->isPlayer) {
            for (auto& other : scene.getObjects()) {
                if (other != selected) {
                    other->isPlayer = false;
                }
            }
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Delete object
    if (ImGui::Button("Delete Object")) {
        ImGui::OpenPopup("Confirm Delete");
    }
    drawDeleteConfirmation(scene);

    ImGui::End();
}

void Gui::drawDeleteConfirmation(Scene& scene) {
    if (ImGui::BeginPopupModal("Confirm Delete", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete this object?");
        if (ImGui::Button("Yes")) {
            scene.deleteObject(scene.getSelectedObject()->name);
            scene.clearSelection();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void Gui::drawLoadScenePopup(Scene& scene) {
    static size_t selectedSceneIndex = 0;
    std::vector<std::string> scenes = scene.getSceneNames();

    if (ImGui::BeginPopupModal("Load Scene Popup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Select a scene to load:");

        if (!scenes.empty()) {
            if (selectedSceneIndex >= scenes.size()) {
                selectedSceneIndex = 0;
            }
            
            if (ImGui::BeginCombo("##SceneCombo", scenes[selectedSceneIndex].c_str())) {
                for (size_t i = 0; i < scenes.size(); ++i) {
                    bool isSelected = (selectedSceneIndex == i);
                    if (ImGui::Selectable(scenes[i].c_str(), isSelected)) {
                        selectedSceneIndex = i;
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            if (ImGui::Button("Load")) {
                scene.clear();
                scene.loadScene(scenes[selectedSceneIndex]);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
        } else {
            ImGui::Text("No scenes available.");
            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }
}

void Gui::drawSaveScenePopup(Scene& scene) {
    static char saveFileName[128] = "";

    if (ImGui::BeginPopupModal("Save Scene Popup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Filename", saveFileName, IM_ARRAYSIZE(saveFileName));

        if (ImGui::Button("Save")) {
            if (scene.saveScene(saveFileName)) {
                ImGui::CloseCurrentPopup();
            } else {
                // Handle save error
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void Gui::drawPlaytestUI() {
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 10, 10), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    ImGui::Begin("PlaytestLabel", nullptr, flags);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.2f, 1.0f), "Playtest");
    ImGui::End();
}

