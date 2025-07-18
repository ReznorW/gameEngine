#include <imgui.h>
#include <fstream>
#include <sstream>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glm/gtc/type_ptr.hpp>

#include "imgui_markdown.h"

#include "gui.hpp"
#include "mode.hpp"
#include "object.hpp"
#include "mesh.hpp"
#include "script.hpp"

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
void Gui::drawMainMenu(Window& window, Scene& scene, std::unique_ptr<Scene>& playScene, Camera& camera, Camera& playCamera, Mode& mode, bool& drawOBB) {
    if (ImGui::BeginMainMenuBar()) {
        // File Menu
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Crtl + N")) {
                scene.clear();
            }
            if (ImGui::MenuItem("Open", "Crtl + O")) {
                openLoadScenePopup = true; 
            }
            if (ImGui::MenuItem("Save As", "Crtl + Shift + S")) {
                openSaveScenePopup = true;
            }
            if (ImGui::MenuItem("Save", "Ctrl + S")) {
                const std::string& sceneName = scene.getName();
                if (!sceneName.empty()) {
                    scene.saveScene(sceneName);
                } else {
                    openSaveScenePopup = true;
                }
            }
            if (ImGui::MenuItem("Exit", "Ctrl + Q")) {
                glfwSetWindowShouldClose(window.getGLFWwindow(), true);
            }
            ImGui::EndMenu();
        }

        // Edit Menu
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("New Object", "C")) {
                std::string objName = "NewObj" + std::to_string(scene.getObjectCount());
                scene.addObject(objName, std::make_shared<Object>(objName, "cube", "default.jpg", "default", "", scene.getResources()));
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
            if (ImGui::MenuItem("Save as Mesh", "Ctrl + M")) {
                Object* selected = scene.getSelectedObject();
                if (selected) {
                    std::vector<Object*> objs;
                    getDescendants(selected, objs);
                    std::string filepath = "assets/models/" + selected->name + ".vert";
                    saveMesh(selected->name, *combineMeshes(selected->name, objs), filepath, scene);
                }
            }
            ImGui::EndMenu();
        }

        // View Menu
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Script Editor")) {
                mode = Mode::ScriptEditor;
            }
            ImGui::EndMenu();
        }

        // Run Menu
        if (ImGui::BeginMenu("Run")) {
            if (ImGui::MenuItem("Playtest", "R")) {
                if (mode == Mode::SceneEditor) {
                    mode = Mode::Playtest;
                    playScene = std::make_unique<Scene>(scene);
                    playScene->clearSelection();
                    for (auto& obj : playScene->getObjects()) {
                        if (obj->isPlayer) {
                            playCamera.position = obj->transform.position;
                            playCamera.yaw = -obj->transform.rotation.y;
                            playCamera.pitch = obj->transform.rotation.x;
                            playCamera.updateCameraVectors();
                        }
                    }
                }
            }
            ImGui::EndMenu();
        }

        // Settings Menu
        if (ImGui::BeginMenu("Settings")) {
            ImGui::MenuItem("Show OBBs", nullptr, &drawOBB);
            ImGui::EndMenu();
        }

        // FPS counter
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 100.0f);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

        ImGui::EndMainMenuBar();
    }
}

void Gui::drawSidebar(Scene& scene) {
    ImGui::SetNextWindowPos(ImVec2(0, 20));
    ImGui::SetNextWindowSize(ImVec2(200, ImGui::GetIO().DisplaySize.y - 20));
    ImGui::Begin("Objects", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    for (auto& obj : scene.getObjects()) {
        if (obj->parent == nullptr) {
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
            drawObjectTree(*child, scene);
        }
        ImGui::TreePop();
    }
}

void Gui::drawPlaytestUI(Scene& scene) {
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 10, 10), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.35f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    ImGui::Begin("PlaytestLabel", nullptr, flags);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.2f, 1.0f), "Playtest");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Objects: %d", scene.getObjectCount());
    ImGui::End();
}

void Gui::drawWelcomeScreen(Context& context) {
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(displaySize);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoScrollbar |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus |
                                    ImGuiWindowFlags_NoNav;

    ImGui::Begin("WelcomeScreen", nullptr, window_flags);

    ImVec2 cardSize(400, 300);
    ImVec2 cardPos((displaySize.x - cardSize.x) * 0.5f, (displaySize.y - cardSize.y) * 0.5f);
    ImGui::SetCursorPos(cardPos);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24, 24));

    ImGui::BeginChild("WelcomeCard", cardSize, true, ImGuiWindowFlags_NoScrollbar);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::SetCursorPosX((cardSize.x - ImGui::CalcTextSize("Welcome to the Vertex Game Engine").x) * 0.5f);
    ImGui::TextColored(ImVec4(1, 1, 1, 1), "Welcome to the Vertex Game Engine");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Dummy(ImVec2(0, ImGui::GetTextLineHeight()));
    
    ImGui::Spacing();
    ImGui::Spacing();

    float buttonWidth = 200;
    float buttonHeight = 40;
    float buttonX = (cardSize.x - buttonWidth) * 0.5f;

    ImGui::SetCursorPosX(buttonX);
    if (ImGui::Button("Create New Project", ImVec2(buttonWidth, buttonHeight))) {
        // TODO: Make projects

        context.currentMode = Mode::SceneEditor;
    }

    ImGui::Spacing();

    ImGui::SetCursorPosX(buttonX);
    if (ImGui::Button("Load Project", ImVec2(buttonWidth, buttonHeight))) {
        openLoadScenePopup = true;
    }

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::End();
    ImGui::PopStyleVar(3);
}

void Gui::drawScriptEditor(Context& context) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_MenuBar |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("Script Editor", nullptr, window_flags);

    static std::shared_ptr<Script> currentScript = nullptr;
    static std::string currentScriptContent = "";
    static bool dirty = false;

    // Menu Bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Script")) {
                currentScript = std::make_shared<Script>("new_script.lua");
                currentScriptContent = currentScript->getSource();
                dirty = true;
                context.editorScene->getResources()->addScript(currentScript); // If you support adding scripts at runtime
            }
            if (ImGui::MenuItem("Save", nullptr, false, dirty && currentScript != nullptr)) {
                currentScript->updateSource(currentScriptContent);
                currentScript->saveToFile();

                std::vector<Object*> matchingObjects;
                for (auto& object : context.editorScene->getObjects()) {
                    if (object->script && object->script->getName() == currentScript->getName()) {
                        matchingObjects.push_back(object);
                    }
                }

                for (auto& object : matchingObjects) {
                    object->script = currentScript;
                }

                dirty = false;
            }
            if (ImGui::MenuItem("Reload", nullptr, false, currentScript != nullptr)) {
                currentScriptContent = currentScript->getSource();
                dirty = false;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Scene Editor")) {
                context.currentMode = Mode::SceneEditor;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("View Documentation")) {
                if (documentation.empty()) {
                    std::ifstream file("docs/Documentation.md");
                    if (file) {
                        std::stringstream buffer;
                        buffer << file.rdbuf();
                        documentation = buffer.str();
                    } else {
                        documentation = "Failed to load Documentation.md.";
                    }
                }
                openHelpPopup = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    // Layout
    ImGui::Columns(2, nullptr, true);
    ImGui::SetColumnWidth(0, 200);

    // Script list
    ImGui::BeginChild("Scripts", ImVec2(0, 0), true);
    auto scripts = context.editorScene->getResources()->getScripts();
    for (const std::shared_ptr<Script>& script : scripts) {
        ImGui::PushID(script->getName().c_str());

        bool selected = (currentScript == script);
        if (ImGui::Selectable(script->getName().c_str(), selected)) {
            currentScript = script;
            currentScriptContent = currentScript->getSource();
            dirty = false;
        }

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Rename")) {
                openRenameScriptPopup = true;
                scriptToRename = script;
            }
            if (ImGui::MenuItem("Delete")) {
                context.editorScene->getResources()->deleteScript(script->getName());
                if (currentScript == script)
                    currentScript = nullptr;
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    ImGui::EndChild();

    ImGui::NextColumn();

    // Text editor
    ImGui::BeginChild("Script Editor Area", ImVec2(0, 0), true);

    if (dirty) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Unsaved changes");
    }

    if (currentScript) {
        ImVec2 editorSize = ImVec2(-1, ImGui::GetContentRegionAvail().y * 0.7f);
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackResize;
        if (InputTextMultilineStdString("##LuaText", currentScriptContent, editorSize, flags)) {
            dirty = true;
        }

        // Error log
        ImGui::Separator();
        ImGui::Text("Error Log:");
        ImGui::BeginChild("LuaErrorTerminal", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

        const std::vector<std::string>& errorLog = currentScript->getErrorLog();
        if (!errorLog.empty()) {
            for (const auto& line : errorLog) {
                ImGui::TextUnformatted(line.c_str());
            }
        } else {
            ImGui::TextDisabled("No errors.");
        }

        ImGui::EndChild();
    } else {
        ImGui::Text("No script selected.");
    }

    ImGui::EndChild();

    ImGui::Columns(1);
    ImGui::End();
}

// === Popup rendering ===
void Gui::drawPopups(Context& context) {
    Scene& scene = *context.editorScene;

    Object* selected = scene.getSelectedObject();
    if (selected && context.currentMode == Mode::SceneEditor) {
        drawObjectPropertiesPopup(scene, selected);
    }

    if (openLoadScenePopup) {
        ImGui::OpenPopup("Load Scene Popup");
        openLoadScenePopup = false;
    }
    drawLoadScenePopup(scene, context.currentMode);

    if (openSaveScenePopup) {
        ImGui::OpenPopup("Save Scene Popup");
        openSaveScenePopup = false;
    }
    drawSaveScenePopup(scene);

    if (openDeleteConfirmationPopup) {
        ImGui::OpenPopup("Confirm Delete");
        openDeleteConfirmationPopup = false;
    }
    drawDeleteConfirmationPopup(scene);

    if (openRenameScriptPopup) {
        ImGui::OpenPopup("Rename Script Popup");
        openRenameScriptPopup = false;
    }
    drawRenameScriptPopup(context);

    if(openHelpPopup && (context.currentMode == Mode::ScriptEditor)) {
        drawDocumentationPopup();
    }
}

void Gui::drawObjectPropertiesPopup(Scene& scene, Object* selected) {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 350, 20));
    ImGui::SetNextWindowSize(ImVec2(350, io.DisplaySize.y));

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
            auto meshes = scene.getResources()->getMeshes();
            for (const std::shared_ptr<Mesh>& mesh : meshes) {
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
            auto shaders = scene.getResources()->getShaders();
            for (const std::shared_ptr<Shader>& shader : shaders) {
                const std::string& shaderName = shader->getName();
                bool isSelected = (selected->shader == shader);
                if (ImGui::Selectable(shaderName.c_str(), isSelected)) {
                    selected->shader = shader;
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Text("Material Properties");
        ImGui::SliderFloat("Ambient",  &selected->material.ambient,  0.0f, 1.0f);
        ImGui::SliderFloat("Specular", &selected->material.specular, 0.0f, 2.0f);
        ImGui::SliderFloat("Shininess", &selected->material.shininess, 1.0f, 128.0f);

        // Texture selector
        std::string currentTextureName = selected->texture ? selected->texture->getName() : "None";

        if (ImGui::BeginCombo("Texture", currentTextureName.c_str())) {
            auto textures = scene.getResources()->getTextures();
            for (const std::shared_ptr<Texture>& tex : textures) {
                const std::string& texName = tex->getName();
                bool isSelected = (selected->texture == tex);
                ImGui::PushID(texName.c_str());
                ImGui::Image(tex->getID(), ImVec2(16, 16));
                ImGui::SameLine();
                if (ImGui::Selectable(texName.c_str(), isSelected)) {
                    selected->texture = tex;
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
                ImGui::PopID();
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
                    break;
                }
            }
        }
    }

    ImGui::Checkbox("Collisions", &selected->hasCollisions);
    ImGui::Checkbox("Moveable", &selected->isMoveable);
    ImGui::Checkbox("Gravity", &selected->hasGravity);

    // Script selector
    std::string currentScript = selected->script ? selected->script->getName() : "None";

    if (ImGui::BeginCombo("Script", currentScript.c_str())) {
        bool isSelected = (selected->script == nullptr);
        if (ImGui::Selectable("None", isSelected)) {
            selected->script = nullptr;
        }
        if (isSelected) {
            ImGui::SetItemDefaultFocus();
        }

        auto scripts = scene.getResources()->getScripts();
        for (const std::shared_ptr<Script>& script : scripts) {
            const std::string& scriptName = script->getName();
            bool isSelected = (scriptName == currentScript);
            if (ImGui::Selectable(scriptName.c_str(), isSelected)) {
                selected->script = script;
                script->setOwner(selected);
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Delete object
    if (ImGui::Button("Delete Object")) {
        openDeleteConfirmationPopup = true;
    }

    ImGui::End();
}

void Gui::drawLoadScenePopup(Scene& scene, Mode& mode) {
    static size_t selectedSceneIndex = 0;
    static bool initialized = false;
    std::vector<std::string> scenes = scene.getResources()->getSceneNames();

    if (ImGui::BeginPopupModal("Load Scene Popup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Select a scene to load:");

        if (!scenes.empty()) {
            if (!initialized) {
                std::string current = scene.getName();
                for (size_t i = 0; i < scenes.size(); ++i) {
                    if (scenes[i] == current) {
                        selectedSceneIndex = i;
                        break;
                    }
                }
                initialized = true;
            }

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
                if (mode != Mode::SceneEditor) {
                    mode = Mode::SceneEditor;
                }
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
    static bool popupJustClosed = false;

    if (ImGui::BeginPopupModal("Save Scene Popup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        popupJustClosed = false;
        ImGui::InputText("Filename", saveFileName, IM_ARRAYSIZE(saveFileName));

        if (ImGui::Button("Save")) {
            if (scene.saveScene(saveFileName)) {
                ImGui::CloseCurrentPopup();
                popupJustClosed = true;
            } else {
                // Handle save error
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
            popupJustClosed = true;
        }

        ImGui::EndPopup();
    } else if (popupJustClosed) {
        saveFileName[0] = '\0';
        popupJustClosed = false;
    }
}

void Gui::drawDeleteConfirmationPopup(Scene& scene) {
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

void Gui::drawRenameScriptPopup(Context& context) {
    static char nameBuffer[128] = "";
    static bool initialized = false;

    if (scriptToRename && ImGui::BeginPopupModal("Rename Script Popup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (!initialized) {
            std::strncpy(nameBuffer, scriptToRename->getName().c_str(), sizeof(nameBuffer));
            nameBuffer[sizeof(nameBuffer) - 1] = '\0';
            initialized = true;
        }

        ImGui::InputText("New Name", nameBuffer, IM_ARRAYSIZE(nameBuffer));

        if (ImGui::Button("Rename")) {
            std::string newName(nameBuffer);
            if (!newName.empty() && newName != scriptToRename->getName()) {
                scriptToRename->setName(newName);
            }
            scriptToRename = nullptr;
            initialized = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            scriptToRename = nullptr;
            initialized = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    } else {
        initialized = false;
    }
}

void Gui::drawDocumentationPopup() {
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Documentation", &openHelpPopup)) {
        static ImGui::MarkdownConfig config;

        ImGui::BeginChild("DocScroll", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::Markdown(documentation.c_str(), documentation.size(), config);
        ImGui::EndChild();
    }
    ImGui::End();
}

// === ImGui utils ===
bool InputTextMultilineStdString(const char* label, std::string& str, const ImVec2& size, ImGuiInputTextFlags flags) {
    flags |= ImGuiInputTextFlags_CallbackResize;

    if (str.empty()) {
        str = "";
    }

    bool changed = ImGui::InputTextMultiline(label, (char*)str.c_str(), str.capacity() + 1, size, flags, InputTextCallback, (void*)&str);

    return changed;
}

int InputTextCallback(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        std::string* str = reinterpret_cast<std::string*>(data->UserData);
        IM_ASSERT(str != nullptr);
        str->resize(data->BufTextLen);
        data->Buf = (char*)str->c_str();
    }
    return 0;
}
